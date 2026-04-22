#include "hmm_tagger.hpp"
#include <cmath>
#include <limits>
#include <vector>
#include <algorithm>


namespace hmm {

    std::vector<std::pair<TipoPalabra, double>> tagWithConfidence(
        const std::vector<std::string>& tokens,
        const CorpusStats& stats
    ) {
        int T = tokens.size();
        int N = MAX_TAGS;

        // Matrices de log-probabilidades y backpointers
        std::vector<std::vector<double>> viterbi(T, std::vector<double>(N, -std::numeric_limits<double>::infinity()));
        std::vector<std::vector<int>> backpointer(T, std::vector<int>(N, -1));

        // Inicialización
        for (int s = 0; s < N; ++s) {
            double prior = std::log(stats.getPrior(static_cast<TipoPalabra>(s)));
            double em = std::log(stats.getEmission(tokens[0], static_cast<TipoPalabra>(s)));
            viterbi[0][s] = prior + em;
        }

        // Recursión
        for (int t = 1; t < T; ++t) {
            for (int s = 0; s < N; ++s) {
                double em = std::log(stats.getEmission(tokens[t], static_cast<TipoPalabra>(s)));
                double max_prob = -std::numeric_limits<double>::infinity();
                int best_prev = -1;
                for (int prev = 0; prev < N; ++prev) {
                    double trans = std::log(stats.getTransition(static_cast<TipoPalabra>(prev), static_cast<TipoPalabra>(s)));
                    double prob = viterbi[t-1][prev] + trans + em;
                    if (prob > max_prob) {
                        max_prob = prob;
                        best_prev = prev;
                    }
                }
                viterbi[t][s] = max_prob;
                backpointer[t][s] = best_prev;
            }
        }

        // Terminación: mejor etiqueta final
        double max_final = -std::numeric_limits<double>::infinity();
        int best_last = 0;
        for (int s = 0; s < N; ++s) {
            if (viterbi[T-1][s] > max_final) {
                max_final = viterbi[T-1][s];
                best_last = s;
            }
        }

        // Backtrack
        std::vector<int> best_tags(T);
        best_tags[T-1] = best_last;
        for (int t = T-2; t >= 0; --t) {
            best_tags[t] = backpointer[t+1][best_tags[t+1]];
        }

        // Construir resultado con confianzas (uso la emisión como confianza individual)
        std::vector<std::pair<TipoPalabra, double>> result;
        result.reserve(T);
        for (int t = 0; t < T; ++t) {
            TipoPalabra tag = static_cast<TipoPalabra>(best_tags[t]);
            double conf = stats.getEmission(tokens[t], tag);
            result.emplace_back(tag, conf);
        }
        return result;
    }

    static double logsumexp(double a, double b) {
        if (a == -std::numeric_limits<double>::infinity()) return b;
        if (b == -std::numeric_limits<double>::infinity()) return a;
        return std::max(a, b) + std::log1p(std::exp(-std::abs(a - b)));
    }
    std::vector<std::pair<TipoPalabra, double>> tagWithPosteriorProb(
        const std::vector<std::string>& tokens,
        const CorpusStats& stats
    ) {
        int T = tokens.size();
        int N = MAX_TAGS;
        const double NEG_INF = -std::numeric_limits<double>::infinity();

        // Alpha (forward) y Beta (backward) en log-space
        std::vector<std::vector<double>> alpha(T, std::vector<double>(N, NEG_INF));
        std::vector<std::vector<double>> beta(T, std::vector<double>(N, NEG_INF));

        // Inicializacion forward
        for (int s = 0; s < N; ++s) {
            double prior = std::log(stats.getPrior(static_cast<TipoPalabra>(s)));
            double em = std::log(stats.getEmission(tokens[0], static_cast<TipoPalabra>(s)));
            alpha[0][s] = prior + em;
        }

        // Recursión forward
        for (int t = 1; t < T; ++t) {
            for (int s = 0; s < N; ++s) {
                double em = std::log(stats.getEmission(tokens[t], static_cast<TipoPalabra>(s)));
                double max_sum = NEG_INF;
                for (int prev = 0; prev < N; ++prev) {
                    double trans = std::log(stats.getTransition(static_cast<TipoPalabra>(prev),
                                                                static_cast<TipoPalabra>(s)));
                    double val = alpha[t-1][prev] + trans;
                    max_sum = logsumexp(max_sum, val);
                }
                alpha[t][s] = max_sum + em;
            }
        }

        // Inicialización backward
        for (int s = 0; s < N; ++s) beta[T-1][s] = 0.0; // log(1)

        // Recursión backward
        for (int t = T-2; t >= 0; --t) {
            for (int s = 0; s < N; ++s) {
                double sum = NEG_INF;
                for (int next = 0; next < N; ++next) {
                    double trans = std::log(stats.getTransition(static_cast<TipoPalabra>(s),
                                                                static_cast<TipoPalabra>(next)));
                    double em = std::log(stats.getEmission(tokens[t+1], static_cast<TipoPalabra>(next)));
                    double val = beta[t+1][next] + trans + em;
                    sum = logsumexp(sum, val);
                }
                beta[t][s] = sum;
            }
        }

        // Probabilidad total de la secuencia (log)
        double log_lik = NEG_INF;
        for (int s = 0; s < N; ++s) log_lik = logsumexp(log_lik, alpha[T-1][s]);

        // Para cada posicion, elegir la etiqueta con mayor probabilidad posterior
        std::vector<std::pair<TipoPalabra, double>> result;
        result.reserve(T);
        for (int t = 0; t < T; ++t) {
            double best_prob = NEG_INF;
            int best_tag = 0;
            for (int s = 0; s < N; ++s) {
                double posterior = alpha[t][s] + beta[t][s] - log_lik;
                if (posterior > best_prob) {
                    best_prob = posterior;
                    best_tag = s;
                }
            }
            double confidence = std::exp(best_prob); // en [0,1]
            result.emplace_back(static_cast<TipoPalabra>(best_tag), confidence);
        }
        return result;
    }

} // namespace hmm
