#include "pattern_repository.hpp"
#include "db_manager.hpp"
#include <sstream>

void PatternRepository::save(const Patron& patron) {
    sqlite3* db = Database::instance().getHandle();
    if (!db) return;

    std::stringstream seq_ss;
    for (size_t i = 0; i < patron.secuencia.size(); ++i) {
        if (i > 0) seq_ss << ",";
        seq_ss << static_cast<int>(patron.secuencia[i]);
    }
    std::string seq_str = seq_ss.str();

    sqlite3_stmt* stmt;
    const char* sql_check = "SELECT id, frecuencia FROM patrones WHERE tipo_patron = ? AND secuencia = ?";
    if(!prepareStatement(db, sql_check, &stmt))return;
    sqlite3_bind_int(stmt, 1, static_cast<int>(patron.tipo));
    sqlite3_bind_text(stmt, 2, seq_str.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        float freq = static_cast<float>(sqlite3_column_double(stmt, 1));
        sqlite3_finalize(stmt);
        const char* sql_upd = "UPDATE patrones SET frecuencia = ? WHERE id = ?";
        if(!prepareStatement(db, sql_upd, &stmt))return;
        sqlite3_bind_double(stmt, 1, freq + 1.0f);
        sqlite3_bind_int(stmt, 2, sqlite3_column_int(stmt, 0));
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        sqlite3_finalize(stmt);
        const char* sql_ins = "INSERT INTO patrones (tipo_patron, secuencia, frecuencia) VALUES (?,?,?)";
        if(!prepareStatement(db, sql_ins, &stmt))return;
        sqlite3_bind_int(stmt, 1, static_cast<int>(patron.tipo));
        sqlite3_bind_text(stmt, 2, seq_str.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, 1.0f);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

// Helper para convertir string almacenado (ej. "1,2,3") a vector<TipoPalabra>
static std::vector<TipoPalabra> secuenciaFromString(const std::string& seq_str) {
    std::vector<TipoPalabra> result;
    std::stringstream ss(seq_str);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) {
            result.push_back(static_cast<TipoPalabra>(std::stoi(item)));
        }
    }
    return result;
}

std::optional<Patron> PatternRepository::findMatch(const std::vector<TipoPalabra>& secuencia, float& out_similitud) {
    sqlite3* db = Database::instance().getHandle();
    if (!db) return std::nullopt;

    std::string target_seq;
    for (size_t i = 0; i < secuencia.size(); ++i) {
        if (i > 0) target_seq += ",";
        target_seq += std::to_string(static_cast<int>(secuencia[i]));
    }
    sqlite3_stmt* stmt;
    const char* sql = "SELECT tipo_patron, secuencia, frecuencia FROM patrones";
    if (!prepareStatement(db, sql, &stmt)) return std::nullopt;

    std::optional<Patron> best;
    float best_sim = 0.0f;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* db_seq = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        float sim = (target_seq == db_seq) ? 1.0f : 0.0f; // coincidencia exacta
        if (sim > best_sim) {
            best_sim = sim;
            TipoPatron tp = static_cast<TipoPatron>(sqlite3_column_int(stmt, 0));
            std::vector<TipoPalabra> seq_vec = secuenciaFromString(db_seq);
            best = Patron(seq_vec, tp);
        }
    }
    sqlite3_finalize(stmt);
    out_similitud = best_sim;
    return best;
}

std::vector<Patron> PatternRepository::loadAll() {
    sqlite3* db = Database::instance().getHandle();
    std::vector<Patron> patrones;
    if (!db) return patrones;

    sqlite3_stmt* stmt;
    const char* sql = "SELECT tipo_patron, secuencia, frecuencia FROM patrones";
    if (!prepareStatement(db, sql, &stmt)) return patrones;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TipoPatron tp = static_cast<TipoPatron>(sqlite3_column_int(stmt, 0));
        const char* seq_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::vector<TipoPalabra> secuencia = secuenciaFromString(seq_str);
        patrones.emplace_back(secuencia, tp);
    }
    sqlite3_finalize(stmt);
    return patrones;
}
