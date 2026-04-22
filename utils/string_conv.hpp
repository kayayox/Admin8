#ifndef STRING_CONV_HPP
#define STRING_CONV_HPP

#include "../types.hpp"
#include <string>

std::string tipoToString(TipoPalabra tipo);
std::string tiempoToString(Tiempo tiempo);
std::string generoToString(Genero genero);
std::string personaToString(Persona persona);
std::string gradoToString(Grado grado);
std::string cantidadToString(Cantidad cant);
std::string tipoPatronToString(TipoPatron tp);

#endif // STRING_CONV_HPP
