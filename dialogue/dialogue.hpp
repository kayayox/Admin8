#ifndef DIALOGUE_HPP
#define DIALOGUE_HPP

#include "pattern.hpp"
#include "../db/sentence_repository.hpp"
#include <vector>

struct Dialogue {
    Oracion premisa;
    Oracion hipotesis;
    Patron patron;
    float creatividad;
};

class DialogueHistory {
public:
    void addDialogue(const Oracion& premisa, const Oracion& hipotesis, const Patron& patron, float creatividad);
    const std::vector<Dialogue>& getHistory() const { return history_; }
    float getThresholdCreativity() const { return threshold_creativity_; }

private:
    std::vector<Dialogue> history_;
    float threshold_creativity_ = 0.5f;
    void updateThreshold();
};

// Generar hipotesis a partir de una premisa y un patron (aun crudo,pieso mejorar la logica)
Oracion generateHypothesis(const Oracion& premisa, const Patron* patron = nullptr, const std::string& keyword = "");

// Calcular creatividad entre premisa e hipotesis
float computeCreativity(const Oracion& premisa, const Oracion& hipotesis, const Patron& patron);

#endif // DIALOGUE_HPP
