/** Modulo aun con pendientes,falta tokenizar signos de puntuacion y mantenerlos
en un vector hot aparte,y tambien tomar en cuenta otros formatos de importancia*/
#include "tokenizer.hpp"
#include <cctype>
#include <cstdio>
#include <algorithm>

// Detecta fechas en varios formatos, verificando que toda la cadena sea consumida
static bool is_date_format(const std::string& token) {
    int y, m, d;
    int n = 0; // cantidad de caracteres consumidos

    // Formato 1: YYYY-MM-DD
    if (sscanf(token.c_str(), "%d-%d-%d%n", &y, &m, &d, &n) == 3 && n == (int)token.size()) {
        return (y >= 1 && y <= 9999 && m >= 1 && m <= 12 && d >= 1 && d <= 31);
    }
    // Formato 2: DD/MM/YYYY
    if (sscanf(token.c_str(), "%d/%d/%d%n", &d, &m, &y, &n) == 3 && n == (int)token.size()) {
        return (y >= 1 && y <= 9999 && m >= 1 && m <= 12 && d >= 1 && d <= 31);
    }
    // Formato 3: YYYY/MM/DD
    if (sscanf(token.c_str(), "%d/%d/%d%n", &y, &m, &d, &n) == 3 && n == (int)token.size()) {
        return (y >= 1 && y <= 9999 && m >= 1 && m <= 12 && d >= 1 && d <= 31);
    }
    // Formato 4: DD.MM.YYYY
    if (sscanf(token.c_str(), "%d.%d.%d%n", &d, &m, &y, &n) == 3 && n == (int)token.size()) {
        return (y >= 1 && y <= 9999 && m >= 1 && m <= 12 && d >= 1 && d <= 31);
    }
    return false;
}

static bool is_number(const std::string& token) {
    char* end = nullptr;
    std::strtod(token.c_str(), &end);
    return (end == token.c_str() + token.size());
}

static TokenType classify_token(const std::string& token) {
    if (is_date_format(token)) return TokenType::DATE;
    if (is_number(token)) return TokenType::NUMBER;
    return TokenType::WORD;
}

static bool is_word_char(unsigned char c) {
    // Cualquier byte no ASCII (acentos, ñ, etc.) es parte de una palabra
    if (c >= 0x80) return true;
    // Para ASCII: letras, dígitos, apóstrofe y guión
    return std::isalnum(c) || c == '\'' || c == '-' || c=='/' || c=='.';
}

static std::string to_lower_utf8(const std::string& s) {
    std::string res;
    res.reserve(s.size());
    for (size_t i = 0; i < s.size(); ) {
        unsigned char c = s[i];
        if (c < 0x80) {
            res.push_back(std::tolower(c));
            ++i;
            continue;
        }
        if ((c & 0xE0) == 0xC0 && i + 1 < s.size()) {
            unsigned char c2 = s[i+1];
            if (c == 0xC3) {
                // Tabla de conversión de mayúsculas acentuadas y Ñ a minúsculas
                switch (c2) {
                    case 0x80: res += "\xC3\xA0"; break; // À -> à
                    case 0x81: res += "\xC3\xA1"; break; // Á -> á
                    case 0x82: res += "\xC3\xA2"; break; // Â -> â
                    case 0x83: res += "\xC3\xA3"; break; // Ã -> ã
                    case 0x84: res += "\xC3\xA4"; break; // Ä -> ä
                    case 0x85: res += "\xC3\xA5"; break; // Å -> å
                    case 0x88: res += "\xC3\xA8"; break; // È -> è
                    case 0x89: res += "\xC3\xA9"; break; // É -> é
                    case 0x8A: res += "\xC3\xAA"; break; // Ê -> ê
                    case 0x8B: res += "\xC3\xAB"; break; // Ë -> ë
                    case 0x8C: res += "\xC3\xAC"; break; // Ì -> ì
                    case 0x8D: res += "\xC3\xAD"; break; // Í -> í
                    case 0x8E: res += "\xC3\xAE"; break; // Î -> î
                    case 0x8F: res += "\xC3\xAF"; break; // Ï -> ï
                    case 0x92: res += "\xC3\xB2"; break; // Ò -> ò
                    case 0x93: res += "\xC3\xB3"; break; // Ó -> ó
                    case 0x94: res += "\xC3\xB4"; break; // Ô -> ô
                    case 0x95: res += "\xC3\xB5"; break; // Õ -> õ
                    case 0x96: res += "\xC3\xB6"; break; // Ö -> ö
                    case 0x99: res += "\xC3\xB9"; break; // Ù -> ù
                    case 0x9A: res += "\xC3\xBA"; break; // Ú -> ú
                    case 0x9B: res += "\xC3\xBB"; break; // Û -> û
                    case 0x9C: res += "\xC3\xBC"; break; // Ü -> ü
                    case 0x91: res += "\xC3\xB1"; break; // Ñ -> ñ
                    default:   res += s.substr(i, 2); break;
                }
            } else {
                res += s.substr(i, 2);
            }
            i += 2;
            continue;
        }
        // Tres o más bytes (emojis, etc.) se copian sin cambios
        if ((c & 0xF0) == 0xE0 && i + 2 < s.size()) {
            res += s.substr(i, 3);
            i += 3;
            continue;
        }
        res += s[i];
        ++i;
    }
    return res;
}

std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    size_t i = 0;
    while (i < input.size()) {
        while (i < input.size() && std::isspace(static_cast<unsigned char>(input[i]))) ++i;
        if (i >= input.size()) break;
        size_t start = i;
        while (i < input.size() && is_word_char(static_cast<unsigned char>(input[i]))) ++i;
        std::string token = input.substr(start, i - start);
        if (token.empty()) continue;
        Token t;
        t.text = to_lower_utf8(token);
        t.type = classify_token(t.text);
        tokens.push_back(t);
    }
    return tokens;
}
