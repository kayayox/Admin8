#ifndef CONTEXT_MEMORY_HPP
#define CONTEXT_MEMORY_HPP

#include "context_info.hpp"
#include "../types.hpp"
#include <string>
#include <optional>

struct ContextRecord {
    TipoPalabra tag;
    float confidence;
    int frequency;
};

class ContextMemory {
public:
    // Consulta la memoria para una palabra y contexto dado.
    static std::optional<ContextRecord> query(const std::string& palabra, const ContextInfo& ctx);

    // Registra o actualiza una entrada tras una corrección del usuario.
    static void recordCorrection(const std::string& palabra, const ContextInfo& ctx,
                                 TipoPalabra tag_correcto, float system_confidence);

    // Refuerzo automatico: se llama cuando el sistema acierta sin intervención.
    static void reinforce(const std::string& palabra, const ContextInfo& ctx, TipoPalabra tag, float current_confidence);

    // Limpieza opcional de entradas obsoletas o poco fiables
    static void cleanup(float min_confidence = 0.3f, int max_days = 30);
};

#endif // CONTEXT_MEMORY_HPP
