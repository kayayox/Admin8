#ifndef PATTERN_REPOSITORY_HPP
#define PATTERN_REPOSITORY_HPP

#include "../dialogue/pattern.hpp"
#include <optional>
#include <vector>

class PatternRepository {
public:
    static void save(const Patron& patron);
    static std::optional<Patron> findMatch(const std::vector<TipoPalabra>& secuencia, float& out_similitud);
    static std::vector<Patron> loadAll();
};

#endif
