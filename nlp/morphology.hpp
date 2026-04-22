#ifndef MORPHOLOGY_HPP
#define MORPHOLOGY_HPP

#include "../types.hpp"
#include <string>

namespace morphology {

    // Deteccion de propiedades gramaticales
    bool isPlural(const std::string& palabra);
    Genero detectGender(const std::string& palabra);
    Tiempo detectTense(const std::string& palabra);
    Persona detectPerson(const std::string& palabra);
    Grado detectAdjectiveDegree(const std::string& palabra);

    // Clasificacion inicial por reglas
    TipoPalabra guessInitialTag(const std::string& palabra);

    // Probabilidad empirica P(tag | sufijo)
    float getSuffixProb(const std::string& palabra, TipoPalabra tag);

    // Utilidad: verificar si termina con un sufijo
    bool endsWith(const std::string& palabra, const std::string& sufijo);

} // namespace morphology

#endif // MORPHOLOGY_HPP
