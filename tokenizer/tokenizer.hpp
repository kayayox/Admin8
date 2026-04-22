#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include "../types.hpp"
#include <string>
#include <vector>

struct Token {
    std::string text;
    TokenType type;
};

std::vector<Token> tokenize(const std::string& input);

#endif // TOKENIZER_HPP
