#ifndef CORPUS_STATS_HPP
#define CORPUS_STATS_HPP

#include "../types.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <array>

class CorpusStats {
public:
    CorpusStats();

    // Inicializar desde archivo de frecuencias
    void loadFromFile(const std::string& filename);

    // Actualizar tras corrección de usuario
    void updateFromCorrection(const std::string& word, TipoPalabra tag,
                              const std::string& prev_word, TipoPalabra prev_tag);

    // Probabilidades
    float getEmission(const std::string& word, TipoPalabra tag) const;
    float getTransition(TipoPalabra from, TipoPalabra to) const;
    float getPrior(TipoPalabra tag) const;

private:
    // Vocabulario: indice numerico para palabras frecuentes
    std::unordered_map<std::string, int> wordToIdx;
    std::vector<std::string> idxToWord;
    // Emisiones: para cada tag, un vector de probabilidades (por indice de palabra)
    std::vector<std::vector<float>> emission;
    // Transiciones: matriz MAX_TAGS x MAX_TAGS
    std::array<std::array<float, MAX_TAGS>, MAX_TAGS> transition;
    // Prior: vector de tamaño MAX_TAGS
    std::vector<float> prior;

    void normalizeEmissions();
    void setDefaultTransitions();
    void setDefaultPrior();

    static constexpr float SMOOTHING_ALPHA = 0.001f;
};

#endif // CORPUS_STATS_HPP
