#ifndef DIALOGUE_REPOSITORY_HPP
#define DIALOGUE_REPOSITORY_HPP

#include "../dialogue/dialogue.hpp"
#include "../types.hpp"
#include <vector>
#include <string>

class DialogueRepository {
public:
    // Guardar un dialogo (premisa e hipotesis ya deben tener id asignado)
    static void saveDialogue(const Oracion& premisa, const Oracion& hipotesis, TipoPatron tipo_patron, float creatividad);

    // Registrar feedback de una palabra
    static void registerFeedback(const std::string& palabra, TipoPalabra tipo_propuesto, TipoPalabra tipo_correcto, int acierto);

    // Construir corpus a partir de los bloques almacenados para inicializar estadisticas
    static std::vector<std::string> buildCorpus();

    // Cargar todo el historial de dialogos desde la base de datos
    static DialogueHistory loadHistory();

private:
    // Helper para obtener o insertar una oración y devolver su ID
    static int getOrCreateOracionId(const Oracion& oracion);
};

#endif // DIALOGUE_REPOSITORY_HPP
