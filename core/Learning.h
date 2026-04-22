#ifndef LEARNING_H_INCLUDED
#define LEARNING_H_INCLUDED

#include "../dialogue/PatternCorrelator.h"
#include "ContextualCorrelator.h"

// Funcion auxiliar para aprender un texto completo con ambos metodos contextuales
void learnText(ContextualCorrelator& ctx, PatternCorrelator& correlator, const std::string& text);

// Limpiar el buffer de entrada
void clearInputBuffer();
void eraserSpace(std::string& line);

#endif // LEARNING_H_INCLUDED
