#ifndef HMM_TAGGER_HPP
#define HMM_TAGGER_HPP

#include "../types.hpp"
#include "corpus_stats.hpp"
#include <vector>
#include <string>

namespace hmm {

    // Etiquetar una secuencia de tokens (version con confianza)
    std::vector<std::pair<TipoPalabra, double>> tagWithConfidence(
        const std::vector<std::string>& tokens,
        const CorpusStats& stats
    );

    // Version simple sin confianza (solo etiquetas)
    inline std::vector<TipoPalabra> tagSentence(
        const std::vector<std::string>& tokens,
        const CorpusStats& stats
    ) {
        auto res = tagWithConfidence(tokens, stats);
        std::vector<TipoPalabra> tags;
        tags.reserve(res.size());
        for (auto& p : res) tags.push_back(p.first);
        return tags;
    }
    std::vector<std::pair<TipoPalabra, double>> tagWithPosteriorProb(
        const std::vector<std::string>& tokens,
        const CorpusStats& stats
    );

} // namespace hmm

#endif // HMM_TAGGER_HPP
