#ifndef PATTERN_HPP
#define PATTERN_HPP

#include "../types.hpp"
#include <vector>

struct Patron {
    std::vector<TipoPalabra> secuencia;
    TipoPatron tipo;

    Patron() : tipo(TipoPatron::SENTENCIAS) {}
    explicit Patron(const std::vector<TipoPalabra>& seq, TipoPatron tp = TipoPatron::SENTENCIAS)
        : secuencia(seq), tipo(tp) {}
};

// Clasificar una oracion a partir de sus bloques (secuencia de tipos)
TipoPatron clasificarOracion(const std::vector<TipoPalabra>& secuencia);

// Crear patron desde una secuencia de tipos
Patron patronFromSecuencia(const std::vector<TipoPalabra>& secuencia);

#endif // PATTERN_HPP
