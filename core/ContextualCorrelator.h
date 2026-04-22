#ifndef CONTEXTUAL_CORRELATOR_H
#define CONTEXTUAL_CORRELATOR_H

#include "../dialogue/PatternCorrelator.h"
#include <string>
#include <vector>
#include <map>

class ContextualCorrelator {
public:
    // Recibe un PatternCorrelator ya inicializado (con su propia BD)
    // Independiente para generar flujos paralelos entre semantica y patrones
    explicit ContextualCorrelator(PatternCorrelator& correlator);

    // Aprende de un texto: para cada palabra, usa hasta dos anteriores como patron previo
    // y la siguiente palabra como patrón siguiente.
    // Si hay menos de dos anteriores, se usan las que existan.
    void learnWithPreviousTwo(const std::string& text);

    // Aprende asociaciones directas: para cada palabra, registra la siguiente (sin contexto)
    // Esto es util para completar el modelo.
    void learnNextWordDirect(const std::string& text);

    // Consulta: dado un contexto de hasta dos palabras anteriores y la palabra actual,
    // devuelve la distribución de palabras siguientes (como patrones de una sola palabra).
    // Ejemplo: queryNext("azul", {"el", "cielo"}) -> posibles siguientes palabras.
    bool queryNext(const std::string& current,
                   const std::vector<std::string>& previousWords,
                   std::vector<std::pair<Pattern, double>>& outcomes);

    // Versión simplificada para dos palabras anteriores exactas.
    bool queryNextWithTwoPrev(const std::string& current,
                              const std::string& prev1,
                              const std::string& prev2,
                              std::vector<std::pair<Pattern, double>>& outcomes);

    // Version con una sola palabra anterior.
    bool queryNextWithOnePrev(const std::string& current,
                              const std::string& prev,
                              std::vector<std::pair<Pattern, double>>& outcomes);

private:
    PatternCorrelator& corr;

    // Construye un Pattern a partir de un vector de palabras (con peso 1.0 cada una)
    Pattern makePattern(const std::vector<std::string>& words) const;
};

#endif
