#ifndef NLP_API_H
#define NLP_API_H

#include <string>
#include <vector>
#include <memory>

// Resultado de clasificación de una palabra
struct WordInfo {
    std::string word;
    std::string tipo;               // "Sustantivo", "Verbo", etc.
    float confianza;                // 0..1
    std::string significado;
    // atributos morfológicos
    std::string cantidad;           // "Singular", "Plural", ""
    std::string tiempo;             // "Pasado", "Presente", "Futuro", ""
    std::string genero;             // "Masculino", "Femenino", "Neutro", ""
    std::string persona;            // "Primera", "Segunda", "Tercera", ""
    std::string grado;              // "Positivo", "Superlativo", ...
};

// Resultado de predicción de siguiente palabra
struct Prediction {
    std::string word;
    double probability;
};

// Clase principal del motor NLP
class NLPEngine {
public:
    // Constructor: inicializa internamente pero no abre BD
    NLPEngine();
    ~NLPEngine();

    // Inicializa las bases de datos SQLite (debe llamarse antes de cualquier otra operacion)
    //   semanticDbPath: base de datos semántica (palabras, oraciones, patrones, dialogos)
    //   correlatorDbPath: base de datos especifica para el correlador de patrones
    // Se mantienen dos archivos por separación de responsabilidades y rendimiento.
    bool initialize(const std::string& semanticDbPath, const std::string& correlatorDbPath);

    void shutdown();

    // Aprende de un texto completo (usa contexto de hasta dos palabras anteriores + trigramas)
    void learnText(const std::string& text);

    // Clasifica una oracion completa (devuelve informacion de cada palabra)
    std::vector<WordInfo> classifySentence(const std::string& sentence);

    // Clasifica una sola palabra (contexto opcional)
    WordInfo classifyWord(const std::string& word, const std::string& context = "");

    // Predice la(s) siguiente(s) palabra(s) mas probables dada la palabra actual y contexto previo
    // previousWords: lista de hasta dos palabras anteriores (la más lejana primero)
    std::vector<Prediction> predictNextWords(const std::string& currentWord,
                                             const std::vector<std::string>& previousWords);

    // Version con una palabra anterior
    std::vector<Prediction> predictNextWordWithOnePrev(const std::string& currentWord,
                                                       const std::string& previousWord);

    // Version con dos palabras anteriores
    std::vector<Prediction> predictNextWordWithTwoPrev(const std::string& currentWord,
                                                       const std::string& prev1,
                                                       const std::string& prev2);

    // Genera una hipotesis de respuesta a partir de una premisa (oracion)
    std::string generateHypothesis(const std::string& premiseSentence);

    // Obtiene informacion completa de una palabra (desde BD o clasificada sobre la marcha)
    WordInfo getWordInfo(const std::string& word);

    // Permite corregir manualmente la clasificación de una palabra.
    // Actualiza la base de datos, las estadisticas del clasificador y la memoria contextual.
    void correctWord(const std::string& word, const std::string& correctType);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // NLP_API_H
