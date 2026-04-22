#include "pattern.hpp"

TipoPatron clasificarOracion(const std::vector<TipoPalabra>& secuencia) {
    bool tiene_negacion = false;
    bool tiene_pregunta = false;
    for (TipoPalabra t : secuencia) {
        // En un sistema real, se necesitarían las palabras reales.
        // Aquí simulamos con tipos; para mejor precisión se necesitaría el texto.
        if (t == TipoPalabra::ADV) { // asumimos que 'no', 'nunca' son adverbios
            tiene_negacion = true;
        }
        if (t == TipoPalabra::PREG) {
            tiene_pregunta = true;
        }
    }
    if (tiene_pregunta && secuencia.size() > 3) return TipoPatron::PREGUNTA_COMP;
    if (tiene_pregunta) return TipoPatron::PREGUNTA_SIMP;
    if (tiene_negacion && secuencia.size() > 3) return TipoPatron::NEGACION_COMP;
    if (tiene_negacion) return TipoPatron::NEGACION_SIMP;
    if (secuencia.size() > 3) return TipoPatron::AFIRMACION_COMP;
    return TipoPatron::AFIRMACION_SIMP;
}

Patron patronFromSecuencia(const std::vector<TipoPalabra>& secuencia) {
    return Patron(secuencia, clasificarOracion(secuencia));
}
