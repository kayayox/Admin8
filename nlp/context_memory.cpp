#include "context_memory.hpp"
#include "../db/db_manager.hpp"
#include <sqlite3.h>
#include <iostream>
#include <cmath>

std::optional<ContextRecord> ContextMemory::query(const std::string& palabra, const ContextInfo& ctx) {
    sqlite3* db = Database::instance().getHandle();
    if (!db) return std::nullopt;

    std::string hash = ctx.getContextHash();  // o getCompactHash() segun preferencia
    sqlite3_stmt* stmt;
    const char* sql = "SELECT tag, confidence, frequency FROM context_memory WHERE palabra = ? AND context_hash = ?";
    if (!prepareStatement(db, sql, &stmt)) return std::nullopt;
    sqlite3_bind_text(stmt, 1, palabra.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hash.c_str(), -1, SQLITE_STATIC);

    ContextRecord record;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        record.tag = static_cast<TipoPalabra>(sqlite3_column_int(stmt, 0));
        record.confidence = static_cast<float>(sqlite3_column_double(stmt, 1));
        record.frequency = sqlite3_column_int(stmt, 2);
        sqlite3_finalize(stmt);
        return record;
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

void ContextMemory::recordCorrection(const std::string& palabra, const ContextInfo& ctx,
                                     TipoPalabra tag_correcto, float system_confidence) {
    sqlite3* db = Database::instance().getHandle();
    if (!db) return;

    std::string hash = ctx.getContextHash();
    sqlite3_stmt* stmt;

    // Verificar si ya existe
    const char* sql_check = "SELECT id, confidence, frequency FROM context_memory WHERE palabra = ? AND context_hash = ?";
    if (!prepareStatement(db, sql_check, &stmt)) return;
    sqlite3_bind_text(stmt, 1, palabra.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hash.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        float old_conf = static_cast<float>(sqlite3_column_double(stmt, 1));
        int old_freq = sqlite3_column_int(stmt, 2);
        sqlite3_finalize(stmt);

        // Nueva confianza = media ponderada -> mayor peso a la corrección humana
        float new_conf = (old_conf * old_freq + 0.95f) / (old_freq + 1);
        int new_freq = old_freq + 1;

        const char* sql_upd = "UPDATE context_memory SET tag = ?, confidence = ?, frequency = ?, last_seen = CURRENT_TIMESTAMP WHERE id = ?";
        if (!prepareStatement(db, sql_upd, &stmt)) return;
        sqlite3_bind_int(stmt, 1, static_cast<int>(tag_correcto));
        sqlite3_bind_double(stmt, 2, new_conf);
        sqlite3_bind_int(stmt, 3, new_freq);
        sqlite3_bind_int(stmt, 4, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        sqlite3_finalize(stmt);
        // Insertar nueva entrada con confianza alta (0.95)
        const char* sql_ins = "INSERT INTO context_memory (palabra, context_hash, tag, confidence, frequency) VALUES (?,?,?,?,?)";
        if (!prepareStatement(db, sql_ins, &stmt)) return;
        sqlite3_bind_text(stmt, 1, palabra.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hash.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, static_cast<int>(tag_correcto));
        sqlite3_bind_double(stmt, 4, 0.95);
        sqlite3_bind_int(stmt, 5, 1);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

void ContextMemory::reinforce(const std::string& palabra, const ContextInfo& ctx,
                              TipoPalabra tag, float current_confidence) {
    sqlite3* db = Database::instance().getHandle();
    if (!db) return;

    std::string hash = ctx.getContextHash();
    sqlite3_stmt* stmt;
    const char* sql_check = "SELECT id, confidence, frequency FROM context_memory WHERE palabra = ? AND context_hash = ?";
    if (!prepareStatement(db, sql_check, &stmt)) return;
    sqlite3_bind_text(stmt, 1, palabra.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hash.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        float old_conf = static_cast<float>(sqlite3_column_double(stmt, 1));
        int old_freq = sqlite3_column_int(stmt, 2);
        sqlite3_finalize(stmt);

        float new_conf = (old_conf * old_freq + current_confidence) / (old_freq + 1);
        int new_freq = old_freq + 1;

        const char* sql_upd = "UPDATE context_memory SET tag = ?, confidence = ?, frequency = ?, last_seen = CURRENT_TIMESTAMP WHERE id = ?";
        if (!prepareStatement(db, sql_upd, &stmt)) return;
        sqlite3_bind_int(stmt, 1, static_cast<int>(tag));
        sqlite3_bind_double(stmt, 2, new_conf);
        sqlite3_bind_int(stmt, 3, new_freq);
        sqlite3_bind_int(stmt, 4, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        sqlite3_finalize(stmt);
        // No existe: la creamos con la confianza actual y la etiqueta proporcionada
        const char* sql_ins = "INSERT INTO context_memory (palabra, context_hash, tag, confidence, frequency) VALUES (?,?,?,?,?)";
        if (!prepareStatement(db, sql_ins, &stmt)) return;
        sqlite3_bind_text(stmt, 1, palabra.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hash.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, static_cast<int>(tag));
        sqlite3_bind_double(stmt, 4, current_confidence);
        sqlite3_bind_int(stmt, 5, 1);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

void ContextMemory::cleanup(float min_confidence, int max_days) {
    sqlite3* db = Database::instance().getHandle();
    if (!db) return;
    const char* sql = "DELETE FROM context_memory WHERE confidence < ? OR julianday('now') - julianday(last_seen) > ?";
    sqlite3_stmt* stmt;
    if (!prepareStatement(db, sql, &stmt)) return;
    sqlite3_bind_double(stmt, 1, min_confidence);
    sqlite3_bind_int(stmt, 2, max_days);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
