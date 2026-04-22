#ifndef CLASSIFIER_HPP
#define CLASSIFIER_HPP

#include "../core/word.hpp"
#include "corpus_stats.hpp"
#include "context_info.hpp"
#include <vector>
#include <string>

struct ContextInfo;

class Classifier {
public:
    explicit Classifier(CorpusStats& stats);

    // Clasificar una oracion completa (lista de palabras)
    void classifySentence(std::vector<Word>& words, const ContextInfo& ctx);

    // Clasificar una sola palabra (legacy)
    void classifyWord(Word& w, const ContextInfo& ctx);

    // Feedback del usuario para una palabra
    void requestCorrection(Word& w,const ContextInfo& ctx);

private:
    CorpusStats& stats_;

    void updateConfidence(Word& w, bool acierto);
};

#endif // CLASSIFIER_HPP
