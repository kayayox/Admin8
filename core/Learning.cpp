#include "Learning.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <cctype>

// Función auxiliar para aprender un texto completo con ambos métodos contextuales
void learnText(ContextualCorrelator& ctx, PatternCorrelator& correlator, const std::string& text) {
    if (text.empty()) return;
    correlator.learnFromText(text, 1);
    ctx.learnWithPreviousTwo(text);
    ctx.learnNextWordDirect(text);
    std::cout << "Texto aprendido (contexto hasta dos anteriores + directo).\n";
}

// Limpiar el buffer de entrada
void clearInputBuffer() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void eraserSpace(std::string& line){
    // Eliminar espacios al inicio y final
    line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), line.end());
}
