#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>

// Enumeraciones principales
enum class TipoPalabra : uint8_t {
    PRON, ART, ADJT, SUST, VERB, PREG, ADV, SENS, PREP, POSE, RELT, NUM, CONJ, CONT, CUANT, DEMS, DATE, INDEFINIDO
};
constexpr int MAX_TAGS = 20;

enum class Cantidad : uint8_t { SING, PLUR, NONE };
enum class Tiempo : uint8_t { PASS, PRES, FUTR, INDETERMINADO };
enum class Genero : uint8_t { MASC, FEME, NEUT };
enum class Grado : uint8_t { COMPARA, SUPERLA, POSIT, INTENS, INTERRG, NEGAT, RELAT, CUANTI, NON };
enum class Persona : uint8_t { PRIM, SEGU, TERC, NIN };

// Tipos de patron
enum class TipoPatron : uint8_t {
    AFIRMACION_SIMP, AFIRMACION_COMP, NEGACION_SIMP, NEGACION_COMP,
    PREGUNTA_SIMP, PREGUNTA_COMP, MIXTO, SENTENCIAS
};

// Tipos de token
enum class TokenType : uint8_t { WORD, NUMBER, DATE };

#endif // TYPES_HPP
