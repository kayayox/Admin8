#include "ContextualCorrelator.h"
#include <sstream>
#include <iostream>

ContextualCorrelator::ContextualCorrelator(PatternCorrelator& correlator)
    : corr(correlator) {}

Pattern ContextualCorrelator::makePattern(const std::vector<std::string>& words) const {
    Pattern pat;
    for (const auto& w : words) {
        pat[w] = 1.0f;  // peso uniforme
    }
    return pat;
}

void ContextualCorrelator::learnWithPreviousTwo(const std::string& text) {
    std::vector<std::string> words;
    std::stringstream ss(text);
    std::string w;
    while (ss >> w) words.push_back(w);

    for (size_t i = 0; i < words.size(); ++i) {
        const std::string& current = words[i];
        // Determinar palabras anteriores (hasta dos)
        std::vector<std::string> prevWords;
        if (i >= 2) {
            prevWords.push_back(words[i-2]);
            prevWords.push_back(words[i-1]);
        } else if (i == 1) {
            prevWords.push_back(words[i-1]);
        }
        // Si no hay anteriores, prevWords queda vacío -> patrón vacío
        if (prevWords.empty()) continue;

        Pattern prevPat = makePattern(prevWords);

        // Palabra siguiente
        if (i + 1 < words.size()) {
            Pattern nextPat = makePattern({words[i+1]});
            corr.record(current, prevPat, nextPat, 1.0f);
        }
    }
}

void ContextualCorrelator::learnNextWordDirect(const std::string& text) {
    std::vector<std::string> words;
    std::stringstream ss(text);
    std::string w;
    while (ss >> w) words.push_back(w);

    for (size_t i = 0; i + 1 < words.size(); ++i) {
        const std::string& current = words[i];
        // Patrón dummy "sin contexto"
        Pattern prevPat = {{"__NO_CONTEXT__", 1.0f}};
        Pattern nextPat = makePattern({words[i+1]});
        corr.record(current, prevPat, nextPat, 1.0f);
    }
}

bool ContextualCorrelator::queryNext(const std::string& current,
                                     const std::vector<std::string>& previousWords,
                                     std::vector<std::pair<Pattern, double>>& outcomes) {
    Pattern prevPat = makePattern(previousWords);
    return corr.query(current, prevPat, outcomes);
}

bool ContextualCorrelator::queryNextWithTwoPrev(const std::string& current,
                                                const std::string& prev1,
                                                const std::string& prev2,
                                                std::vector<std::pair<Pattern, double>>& outcomes) {
    return queryNext(current, {prev2, prev1}, outcomes);
}

bool ContextualCorrelator::queryNextWithOnePrev(const std::string& current,
                                                const std::string& prev,
                                                std::vector<std::pair<Pattern, double>>& outcomes) {
    return queryNext(current, {prev}, outcomes);
}
