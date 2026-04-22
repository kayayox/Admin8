#include "word_repository.hpp"
#include "db_manager.hpp"
#include <sqlite3.h>

void WordRepository::save(const Word& word) {
    sqlite3* db = Database::instance().getHandle();
    if (!db) return;

    sqlite3_stmt* stmt;
    const char* sql_check = "SELECT id FROM palabras WHERE palabra = ?";
    if(!prepareStatement(db, sql_check, &stmt))return;
    sqlite3_bind_text(stmt, 1, word.getPalabra().c_str(), -1, SQLITE_STATIC);
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);

    if (exists) {
        const char* sql_upd = "UPDATE palabras SET significado=?, tipo=?, cantidad=?, tiempo=?, genero=?, grado=?, persona=?, confianza=? WHERE palabra=?";
        if(!prepareStatement(db, sql_upd, &stmt))return;
        sqlite3_bind_text(stmt, 1, word.getSignificado().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, static_cast<int>(word.getTipo()));
        sqlite3_bind_int(stmt, 3, static_cast<int>(word.getCantidad()));
        sqlite3_bind_int(stmt, 4, static_cast<int>(word.getTiempo()));
        sqlite3_bind_int(stmt, 5, static_cast<int>(word.getGenero()));
        sqlite3_bind_int(stmt, 6, static_cast<int>(word.getGrado()));
        sqlite3_bind_int(stmt, 7, static_cast<int>(word.getPersona()));
        sqlite3_bind_double(stmt, 8, word.getConfianza());
        sqlite3_bind_text(stmt, 9, word.getPalabra().c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        const char* sql_ins = "INSERT INTO palabras (palabra, significado, tipo, cantidad, tiempo, genero, grado, persona, confianza) VALUES (?,?,?,?,?,?,?,?,?)";
        if(!prepareStatement(db, sql_ins, &stmt))return;
        sqlite3_bind_text(stmt, 1, word.getPalabra().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, word.getSignificado().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, static_cast<int>(word.getTipo()));
        sqlite3_bind_int(stmt, 4, static_cast<int>(word.getCantidad()));
        sqlite3_bind_int(stmt, 5, static_cast<int>(word.getTiempo()));
        sqlite3_bind_int(stmt, 6, static_cast<int>(word.getGenero()));
        sqlite3_bind_int(stmt, 7, static_cast<int>(word.getGrado()));
        sqlite3_bind_int(stmt, 8, static_cast<int>(word.getPersona()));
        sqlite3_bind_double(stmt, 9, word.getConfianza());
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

bool WordRepository::load(const std::string& palabra, Word& out_word) {
    sqlite3* db = Database::instance().getHandle();
    if (!db) return false;

    sqlite3_stmt* stmt;
    const char* sql = "SELECT significado, tipo, cantidad, tiempo, genero, grado, persona, confianza FROM palabras WHERE palabra = ?";
    if (!prepareStatement(db, sql, &stmt)) return false;
    sqlite3_bind_text(stmt, 1, palabra.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    out_word.setPalabra(palabra);
    out_word.setSignificado(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)), false);
    out_word.setTipo(static_cast<TipoPalabra>(sqlite3_column_int(stmt, 1)), false);
    out_word.setCantidad(static_cast<Cantidad>(sqlite3_column_int(stmt, 2)), false);
    out_word.setTiempo(static_cast<Tiempo>(sqlite3_column_int(stmt, 3)), false);
    out_word.setGenero(static_cast<Genero>(sqlite3_column_int(stmt, 4)), false);
    out_word.setGrado(static_cast<Grado>(sqlite3_column_int(stmt, 5)), false);
    out_word.setPersona(static_cast<Persona>(sqlite3_column_int(stmt, 6)), false);
    out_word.setConfianza(static_cast<float>(sqlite3_column_double(stmt, 7)), false);

    sqlite3_finalize(stmt);
    return true;
}
