#include "dialogue_repository.hpp"
#include "db_manager.hpp"
#include "sentence_repository.hpp"
#include <sqlite3.h>
#include <iostream>

int DialogueRepository::getOrCreateOracionId(const Oracion& oracion) {
    if (oracion.getId() > 0) return oracion.getId();

    Oracion copy = oracion;
    copy.save();
    return copy.getId();
}

void DialogueRepository::saveDialogue(const Oracion& premisa, const Oracion& hipotesis, TipoPatron tipo_patron, float creatividad) {
    sqlite3* db = Database::instance().getHandle();
    if (!db) return;

    int premisa_id = getOrCreateOracionId(premisa);
    int hipotesis_id = getOrCreateOracionId(hipotesis);

    // Verificar si ya existe un diálogo idéntico
    sqlite3_stmt* check;
    const char* sql_check = "SELECT id FROM dialogos WHERE premisa_id = ? AND hipotesis_id = ? AND tipo_patron = ? AND creatividad = ?";
    if(!prepareStatement(db, sql_check, &check))return;
    sqlite3_bind_int(check, 1, premisa_id);
    sqlite3_bind_int(check, 2, hipotesis_id);
    sqlite3_bind_int(check, 3, static_cast<int>(tipo_patron));
    sqlite3_bind_double(check, 4, creatividad);

    if (sqlite3_step(check) == SQLITE_ROW) {
        // Ya existe, no hacer nada
        sqlite3_finalize(check);
        return;
    }
    sqlite3_finalize(check);

    // Insertar nuevo diálogo
    sqlite3_stmt* stmt;
    const char* sql_ins = "INSERT INTO dialogos (premisa_id, hipotesis_id, tipo_patron, creatividad) VALUES (?,?,?,?)";
    if(!prepareStatement(db, sql_ins, &stmt))return;
    sqlite3_bind_int(stmt, 1, premisa_id);
    sqlite3_bind_int(stmt, 2, hipotesis_id);
    sqlite3_bind_int(stmt, 3, static_cast<int>(tipo_patron));
    sqlite3_bind_double(stmt, 4, creatividad);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void DialogueRepository::registerFeedback(const std::string& palabra, TipoPalabra tipo_propuesto, TipoPalabra tipo_correcto, int acierto) {
    sqlite3* db = Database::instance().getHandle();
    if (!db) return;

    // Verificar si ya existe un feedback idéntico
    sqlite3_stmt* check;
    const char* sql_check = "SELECT id FROM feedback WHERE palabra = ? AND tipo_propuesto = ? AND tipo_correcto = ? AND acierto = ?";
    if(!prepareStatement(db, sql_check, &check))return;
    sqlite3_bind_text(check, 1, palabra.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(check, 2, static_cast<int>(tipo_propuesto));
    sqlite3_bind_int(check, 3, static_cast<int>(tipo_correcto));
    sqlite3_bind_int(check, 4, acierto);

    if (sqlite3_step(check) == SQLITE_ROW) {
        sqlite3_finalize(check);
        return;
    }
    sqlite3_finalize(check);

    // Insertar nuevo feedback
    sqlite3_stmt* stmt;
    const char* sql_ins = "INSERT INTO feedback (palabra, tipo_propuesto, tipo_correcto, acierto) VALUES (?,?,?,?)";
    if(!prepareStatement(db, sql_ins, &stmt))return;
    sqlite3_bind_text(stmt, 1, palabra.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, static_cast<int>(tipo_propuesto));
    sqlite3_bind_int(stmt, 3, static_cast<int>(tipo_correcto));
    sqlite3_bind_int(stmt, 4, acierto);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<std::string> DialogueRepository::buildCorpus() {
    sqlite3* db = Database::instance().getHandle();
    std::vector<std::string> corpus;
    if (!db) return corpus;

    sqlite3_stmt* stmt;
    const char* sql = "SELECT bloque_texto FROM bloques ORDER BY oracion_id, posicion";
    if(!prepareStatement(db, sql, &stmt)) return corpus;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* word = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        corpus.emplace_back(word);
    }
    sqlite3_finalize(stmt);
    return corpus;
}

DialogueHistory DialogueRepository::loadHistory() {
    DialogueHistory history;
    sqlite3* db = Database::instance().getHandle();
    if (!db) return history;

    sqlite3_stmt* stmt;
    const char* sql = "SELECT premisa_id, hipotesis_id, tipo_patron, creatividad FROM dialogos ORDER BY timestamp";
    if(!prepareStatement(db, sql, &stmt))return history;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int premisa_id = sqlite3_column_int(stmt, 0);
        int hipotesis_id = sqlite3_column_int(stmt, 1);
        TipoPatron tp = static_cast<TipoPatron>(sqlite3_column_int(stmt, 2));
        float creatividad = static_cast<float>(sqlite3_column_double(stmt, 3));

        Oracion premisa = SentenceRepository::loadById(premisa_id);
        Oracion hipotesis = SentenceRepository::loadById(hipotesis_id);
        // Crear un patrón a partir de la secuencia de tipos de la premisa
        Patron patron = patronFromSecuencia(premisa.getSecuenciaTipos());
        // Forzar el tipo almacenado
        patron.tipo = tp;

        history.addDialogue(premisa, hipotesis, patron, creatividad);
    }
    sqlite3_finalize(stmt);
    return history;
}
