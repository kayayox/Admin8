#ifndef PATTERN_CORRELATOR_H
#define PATTERN_CORRELATOR_H

#include "../utils/PatternUtils.h"
#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>


class PatternCorrelator {
public:
    PatternCorrelator(const std::string& dbPath);
    ~PatternCorrelator();

    // Registra una correlación: dada la palabra actual y un patron anterior,
    // se observa un patron siguiente con un peso determinado.
    void record(const std::string& current, const Pattern& prevPattern, const Pattern& nextPattern, float weight = 1.0f);

    // Consulta la distribución de patrones siguientes para (current, prevPattern).
    // Devuelve true si hay datos, y llena 'outcomes' con pares (patron, probabilidad).
    bool query(const std::string& current, const Pattern& prevPattern, std::vector<std::pair<Pattern, double>>& outcomes);

    // Aprende de un texto: extrae todos los trigramas (prev, curr, next) y los registra
    // usando patrones de una sola palabra con peso 1. (windowSize se ignora, solo se soporta 1).
    void learnFromText(const std::string& text, size_t windowSize = 1);

private:
    sqlite3* db;

    // Caches en memoria
    std::map<std::string, int> wordToId;
    std::map<int, std::string> idToWord;
    std::map<std::string, int> patternSerializedToId;
    std::map<int, Pattern> idToPattern;

    // Sentencias SQL preparadas
    sqlite3_stmt* stmtGetWordId = nullptr;
    sqlite3_stmt* stmtInsertWord = nullptr;
    sqlite3_stmt* stmtGetPatternId = nullptr;
    sqlite3_stmt* stmtInsertPattern = nullptr;
    sqlite3_stmt* stmtFindCorrelation = nullptr;
    sqlite3_stmt* stmtUpdateCorrelation = nullptr;
    sqlite3_stmt* stmtInsertCorrelation = nullptr;
    sqlite3_stmt* stmtGetTotalWeight = nullptr;
    sqlite3_stmt* stmtGetNextPatterns = nullptr;

    int getWordId(const std::string& word);
    int getPatternId(const Pattern& pattern);
    void updateCorrelation(int currWordId, int prevPatternId, int nextPatternId, float weight);
};


#endif
