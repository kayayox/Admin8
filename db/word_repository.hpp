#ifndef WORD_REPOSITORY_HPP
#define WORD_REPOSITORY_HPP

#include "../core/word.hpp"
#include <optional>

class WordRepository {
public:
    static void save(const Word& word);
    static bool load(const std::string& palabra, Word& out_word);
};

#endif // WORD_REPOSITORY_HPP
