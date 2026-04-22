#ifndef CONTEXT_INFO_HPP
#define CONTEXT_INFO_HPP

#include "../types.hpp"
#include <string>
#include <functional>

struct ContextInfo {
    // Palabras literales (normalizadas a minusculas)
    std::string palabra_anterior;
    std::string palabra_siguiente;

    // Etiqueta HMM de la palabra anterior (si ya se clasificó)
    TipoPalabra etiqueta_anterior = TipoPalabra::INDEFINIDO;

    // Propiedades posicionales
    bool es_primera_de_oracion = false;
    bool tiene_mayuscula_inicial = false;
    char puntuacion_anterior = 0;   // '.', ',', ';', etc.

    // Contexto extendido
    std::string texto_previo;

    // Genera una clave unica para este contexto (ignorando texto_previo)
    std::string getContextHash() const {
        // Usamos la palabra anterior y la siguiente como base
        std::string prev = palabra_anterior.empty() ? "<START>" : palabra_anterior;
        std::string next = palabra_siguiente.empty() ? "<END>" : palabra_siguiente;
        // Añadimos información de puntuación y mayúscula para mayor discriminación
        return prev + "|" + next + "|" + std::to_string(es_primera_de_oracion) + "|" +
               std::to_string(tiene_mayuscula_inicial) + "|" + puntuacion_anterior;
    }

    // Version mas compacta usando solo etiqueta anterior y siguiente palabra
    std::string getCompactHash() const {
        std::string prev_tag = std::to_string(static_cast<int>(etiqueta_anterior));
        std::string next = palabra_siguiente.empty() ? "<END>" : palabra_siguiente;
        return prev_tag + "|" + next;
    }
};

#endif
