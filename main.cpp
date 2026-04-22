/** Ejemplo de uso de la Api y prueba inicial*/
#include <sstream>
#include "nlp_api.h"
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <algorithm>
#include <cctype>

// Función auxiliar para mostrar una palabra con sus atributos
void printWordInfo(const WordInfo& info) {
    std::cout << "Palabra: " << info.word << "\n"
              << "  Tipo: " << info.tipo << " (confianza: " << info.confianza << ")\n"
              << "  Significado: " << info.significado << "\n"
              << "  Cantidad: " << info.cantidad << ", Tiempo: " << info.tiempo
              << ", Género: " << info.genero << ", Persona: " << info.persona
              << ", Grado: " << info.grado << "\n";
}

// Funcion para mostrar las predicciones
void printPredictions(const std::vector<Prediction>& preds) {
    if (preds.empty()) {
        std::cout << "  No hay predicciones disponibles.\n";
        return;
    }
    std::cout << "  Predicciones:\n";
    for (size_t i = 0; i < preds.size(); ++i) {
        std::cout << "    " << i+1 << ". '" << preds[i].word
                  << "' (probabilidad: " << preds[i].probability << ")\n";
    }
}

// Aprende todas las lineas no vacias de un archivo
bool learnFromFile(NLPEngine& engine, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Advertencia: No se pudo abrir el archivo '" << filename
                  << "'. Se continuará sin aprendizaje inicial.\n";
        return false;
    }

    std::string line;
    int lineCount = 0;
    while (std::getline(file, line)) {
        // Eliminar espacios al inicio y final
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue; // Línea vacia
        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);

        engine.learnText(line);
        lineCount++;
        std::cout << "Aprendida línea " << lineCount << ": \"" << line << "\"\n";
    }
    std::cout << "Total de " << lineCount << " oraciones aprendidas desde '" << filename << "'.\n";
    return true;
}

// Obtiene la palabra más probable de una lista de predicciones (umbral 0.5)
std::string getBestPrediction(const std::vector<Prediction>& preds, double threshold = 0.5) {
    for (const auto& p : preds) {
        if (p.probability > threshold)
            return p.word;
    }
    return "";
}

// Limpia el buffer de entrada y espera una respuesta válida (s/n)
bool askYesNo(const std::string& question) {
    std::string answer;
    while (true) {
        std::cout << question << " (s/n): ";
        std::getline(std::cin, answer);
        if (answer.empty()) continue;
        char first = std::tolower(answer[0]);
        if (first == 's') return true;
        if (first == 'n') return false;
        std::cout << "Respuesta no válida. Por favor responda 's' o 'n'.\n";
    }
}

int main() {
    // 1. Inicializar el motor NLP
    NLPEngine engine;
    std::string semanticDb = "nlp_semantic.db";
    std::string correlatorDb = "nlp_correlations.db";

    if (!engine.initialize(semanticDb, correlatorDb)) {
        std::cerr << "Error fatal: No se pudo inicializar el motor NLP.\n";
        return 1;
    }
    std::cout << "Motor NLP inicializado correctamente (BD: " << semanticDb << ", " << correlatorDb << ")\n\n";

    // 2. Aprendizaje desde archivo (si existe)
    std::cout << "--- Fase de aprendizaje desde archivo ---\n";
    learnFromFile(engine, "aprende.txt");// Tambien esta este archivo anexado
    std::cout << std::endl;

    // 3. Bucle interactivo de predicción y retroalimentación REAL
    std::cout << "=== MODO INTERACTIVO: Predicción de siguiente palabra ===\n";
    std::cout << "Ingrese una frase inicial (mínimo 2 palabras): ";
    std::string input;
    std::getline(std::cin, input);

    // Validar que la entrada tenga al menos 2 palabras
    std::vector<std::string> words;
    std::stringstream ss(input);
    std::string w;
    while (ss >> w) words.push_back(w);
    if (words.size() < 2) {
        std::cerr << "Error: Se requieren al menos 2 palabras para comenzar.\n";
        engine.shutdown();
        return 1;
    }

    // Parametros del bucle
    const int MAX_ITERATIONS = 5;
    std::string currentPhrase = input;

    for (int iter = 1; iter <= MAX_ITERATIONS; ++iter) {
        std::cout << "\n=== Iteración " << iter << " ===\n";

        // Obtener ultima palabra y la anterior
        std::vector<std::string> tokens;
        std::stringstream ss2(currentPhrase);
        std::string token;
        while (ss2 >> token) tokens.push_back(token);
        if (tokens.size() < 2) break;

        std::string prev1 = tokens[tokens.size() - 2];
        std::string current = tokens.back();

        std::cout << "Contexto: palabra anterior = '" << prev1
                  << "', palabra actual = '" << current << "'\n";

        // Solicitar predicción
        auto predictions = engine.predictNextWordWithOnePrev(current, prev1);
        printPredictions(predictions);

        if (predictions.empty()) {
            std::cout << "No hay predicciones. No se puede continuar en esta iteración.\n";
            continue;
        }

        // Seleccionar la mejor predicción
        std::string predictedWord = getBestPrediction(predictions, 0.5);
        if (predictedWord.empty() && !predictions.empty()) {
            predictedWord = predictions[0].word;
        }
        std::cout << "\nPalabra predicha como siguiente: '" << predictedWord << "'\n";

        bool isCorrect=true;

        // INTERACCIÓN REAL CON EL USUARIO
        if(predictions.size()!=1)isCorrect = askYesNo("¿Es correcta la predicción?");

        std::string finalWord;
        if (isCorrect) {
            finalWord = predictedWord;
            std::cout << "Predicción aceptada. Se aprenderá la secuencia.\n";
        } else {
            std::cout << "Por favor, ingrese la palabra correcta: ";
            std::getline(std::cin, finalWord);
            // Eliminar espacios
            finalWord.erase(0, finalWord.find_first_not_of(" \t\r\n"));
            finalWord.erase(finalWord.find_last_not_of(" \t\r\n") + 1);
            if (finalWord.empty()) {
                std::cerr << "Palabra vacía. Se usará la predicción original para no interrumpir.\n";
                finalWord = predictedWord;
            } else {
                std::cout << "Corrección aplicada: se usará '" << finalWord << "'.\n";
            }
        }

        // Aprender la secuencia correcta: prev1 + current + finalWord
        std::string correctedSentence = prev1 + " " + current + " " + finalWord;
        engine.learnText(correctedSentence);
        std::cout << "Aprendida: \"" << correctedSentence << "\"\n";

        // Actualizar la frase actual añadiendo la palabra (correcta o predicha)
        currentPhrase += " " + finalWord;

        // Mostrar información actualizada de la palabra usada
        WordInfo updatedInfo = engine.getWordInfo(finalWord);
        std::cout << "\nInformación actualizada de la palabra '" << finalWord << "':\n";
        printWordInfo(updatedInfo);
    }

    std::cout << "\n--- Frase final generada (con retroalimentación real) ---\n"
              << currentPhrase << "\n\n";

    // 4. Cierre del motor
    engine.shutdown();
    std::cout << "\nMotor NLP cerrado correctamente. Presione Enter para salir...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

    return 0;
}
