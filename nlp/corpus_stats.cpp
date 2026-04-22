#include "corpus_stats.hpp"
#include "morphology.hpp"
#include <fstream>
#include <cmath>
#include <algorithm>

CorpusStats::CorpusStats() {
    setDefaultTransitions();
    setDefaultPrior();
    emission.resize(MAX_TAGS);
}
/// Se anexa en el modelo un Archivo word_freq.txt para este fin
void CorpusStats::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::string word;
    int freq;
    int idx = 0;
    while (file >> word >> freq && idx < 2000) {
        wordToIdx[word] = idx;
        idxToWord.push_back(word);
        // Asignar emisión inicial con guessInitialTag
        TipoPalabra tag = morphology::guessInitialTag(word);
        emission.resize(MAX_TAGS);
        for (auto& vec : emission) vec.resize(idxToWord.size(), SMOOTHING_ALPHA);
        emission[static_cast<int>(tag)][idx] += 1.0f;
        idx++;
    }
    normalizeEmissions();
}

void CorpusStats::updateFromCorrection(const std::string& word, TipoPalabra tag,
                                       const std::string& prev_word, TipoPalabra prev_tag) {
    // Asegurar que la palabra exista en el vocabulario
    int idx = -1;
    auto it = wordToIdx.find(word);
    if (it != wordToIdx.end()) {
        idx = it->second;
    } else if (idxToWord.size() < 2000) {
        idx = idxToWord.size();
        wordToIdx[word] = idx;
        idxToWord.push_back(word);
        // Ampliar matrices de emision
        for (auto& vec : emission) vec.resize(idxToWord.size(), SMOOTHING_ALPHA);
    }

    if (idx >= 0) {
        // Incrementar emision
        emission[static_cast<int>(tag)][idx] += 1.0f;
        normalizeEmissions();
    }

    // Actualizar transicion si se proporciona palabra anterior
    if (!prev_word.empty()) {
        transition[static_cast<int>(prev_tag)][static_cast<int>(tag)] += 1.0f;
        // Renormalizar fila
        float sum = 0.0f;
        for (int i = 0; i < MAX_TAGS; ++i)
            sum += transition[static_cast<int>(prev_tag)][i];
        if (sum > 0) {
            for (int i = 0; i < MAX_TAGS; ++i)
                transition[static_cast<int>(prev_tag)][i] /= sum;
        }
    }
}

float CorpusStats::getEmission(const std::string& word, TipoPalabra tag) const {
    auto it = wordToIdx.find(word);
    if (it != wordToIdx.end() && it->second < static_cast<int>(emission[0].size())) {
        return emission[static_cast<int>(tag)][it->second];
    }
    return SMOOTHING_ALPHA; // palabra desconocida
}

float CorpusStats::getTransition(TipoPalabra from, TipoPalabra to) const {
    return transition[static_cast<int>(from)][static_cast<int>(to)];
}

float CorpusStats::getPrior(TipoPalabra tag) const {
    return prior[static_cast<int>(tag)];
}

void CorpusStats::normalizeEmissions() {
    if (emission.empty() || emission[0].empty()) return;
    for (int t = 0; t < MAX_TAGS; ++t) {
        float total = 0.0f;
        for (size_t i = 0; i < emission[0].size(); ++i)
            total += emission[t][i];
        if (total == 0.0f) total = 1.0f;
        for (size_t i = 0; i < emission[0].size(); ++i)
            emission[t][i] = (emission[t][i] + SMOOTHING_ALPHA) / (total + SMOOTHING_ALPHA * MAX_TAGS);
    }
}

void CorpusStats::setDefaultTransitions() {
    // Inicializar con valores bajos
    for (int i = 0; i < MAX_TAGS; ++i)
        for (int j = 0; j < MAX_TAGS; ++j)
            transition[i][j] = 0.01f;

    // Transiciones típicas no estandarizadas del español
    transition[static_cast<int>(TipoPalabra::ART)][static_cast<int>(TipoPalabra::SUST)] = 0.6f;
    transition[static_cast<int>(TipoPalabra::ART)][static_cast<int>(TipoPalabra::ADJT)] = 0.2f;
    transition[static_cast<int>(TipoPalabra::ADJT)][static_cast<int>(TipoPalabra::SUST)] = 0.5f;
    transition[static_cast<int>(TipoPalabra::SUST)][static_cast<int>(TipoPalabra::VERB)] = 0.4f;
    transition[static_cast<int>(TipoPalabra::PRON)][static_cast<int>(TipoPalabra::VERB)] = 0.5f;
    transition[static_cast<int>(TipoPalabra::VERB)][static_cast<int>(TipoPalabra::ADV)] = 0.3f;
    transition[static_cast<int>(TipoPalabra::VERB)][static_cast<int>(TipoPalabra::SUST)] = 0.2f;
    transition[static_cast<int>(TipoPalabra::PREP)][static_cast<int>(TipoPalabra::ART)] = 0.4f;
    transition[static_cast<int>(TipoPalabra::PREP)][static_cast<int>(TipoPalabra::PRON)] = 0.3f;

    // Normalizar cada fila
    for (int i = 0; i < MAX_TAGS; ++i) {
        float sum = 0.0f;
        for (int j = 0; j < MAX_TAGS; ++j)
            sum += transition[i][j];
        if (sum > 0) {
            for (int j = 0; j < MAX_TAGS; ++j)
                transition[i][j] /= sum;
        }
    }
}

void CorpusStats::setDefaultPrior() {
    prior.assign(MAX_TAGS, 1.0f / MAX_TAGS);
    // Podríamos ajustar según frecuencias reales, pero dejamos uniforme por ahora
    /// Mejora pendiente
}
