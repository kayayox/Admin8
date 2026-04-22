#include "dialogue.hpp"
#include <algorithm>

void DialogueHistory::addDialogue(const Oracion& premisa, const Oracion& hipotesis, const Patron& patron, float creatividad) {
    history_.push_back({premisa, hipotesis, patron, creatividad});
    updateThreshold();
}

void DialogueHistory::updateThreshold() {
    if (history_.empty()) return;
    float sum = 0.0f;
    for (const auto& d : history_) sum += d.creatividad;
    threshold_creativity_ = sum / history_.size();
}

Oracion generateHypothesis(const Oracion& premisa, const Patron* patron, const std::string& keyword) {
    Oracion hip = premisa; // copia
    if (!patron) {
        // Usar patrón derivado de la premisa
        Patron p = patronFromSecuencia(premisa.getSecuenciaTipos());
        patron = &p;
    }

    switch (patron->tipo) {
        case TipoPatron::PREGUNTA_SIMP:
            hip.addBloque("?", TipoPalabra::INDEFINIDO);
            break;
        case TipoPatron::PREGUNTA_COMP:
            hip.insertarBloqueInicio("qué", TipoPalabra::PREG);
            break;
        case TipoPatron::NEGACION_SIMP:
            hip.insertarNegacion();
            break;
        case TipoPatron::NEGACION_COMP:
            hip.insertarBloqueInicio("nunca", TipoPalabra::ADV);
            break;
        default:
            if (!keyword.empty()) hip.reemplazarSustantivo(keyword);
            break;
    }
    return hip;
}

float computeCreativity(const Oracion& premisa, const Oracion& hipotesis, const Patron& patron) {
    // Simplificado: si comparten algún sustantivo, alta creatividad
    for (const auto& b : premisa.getBloques()) {
        if (b.typo_b == TipoPalabra::SUST) {
            for (const auto& hb : hipotesis.getBloques()) {
                if (hb.typo_b == TipoPalabra::SUST && hb.block == b.block)
                    return 0.9f;
            }
        }
    }
    return 0.5f;
}
