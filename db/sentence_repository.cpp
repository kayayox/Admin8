#include "sentence_repository.hpp"
#include "db_manager.hpp"
#include "../core/word.hpp"
#include <algorithm>
#include <err.h>

Oracion::Oracion(std::vector<Word>& words) {
    for (const auto& w : words) {
        Bloque b;
        b.block = w.getPalabra();
        b.typo_b = w.getTipo();
        bloques_.push_back(b);
        if (w.getTipo() == TipoPalabra::VERB && tiempo_ == Tiempo::INDETERMINADO){
            tiempo_ = w.getTiempo();
            clave_.block = w.getPalabra();
            clave_.typo_b = TipoPalabra::VERB;
            frecuencia_ = 1.0f;
        }
        if (w.getTipo() == TipoPalabra::SUST) {
            clave_.block = w.getPalabra();
            clave_.typo_b = TipoPalabra::SUST;
            frecuencia_ = 1.0f;
        }
    }
    if (clave_.typo_b != TipoPalabra::SUST) {
        for (const auto& b : bloques_) {
            if (b.typo_b == TipoPalabra::VERB) {
                clave_ = b;
                frecuencia_ = 1.2f;
                break;
            }
        }
    }
}

void Oracion::addBloque(const std::string& texto, TipoPalabra tipo) {
    bloques_.push_back({texto, tipo});
}

void Oracion::insertarBloqueInicio(const std::string& texto, TipoPalabra tipo) {
    bloques_.insert(bloques_.begin(), {texto, tipo});
}

void Oracion::insertarNegacion() {
    // Buscar primer verbo e insertar "no" después
    for (size_t i = 0; i < bloques_.size(); ++i) {
        if (bloques_[i].typo_b == TipoPalabra::VERB) {
            bloques_.insert(bloques_.begin() + i + 1, {"no", TipoPalabra::ADV});
            break;
        }
    }
}

void Oracion::reemplazarSustantivo(const std::string& nuevaPalabra) {
    for (auto& b : bloques_) {
        if (b.typo_b == TipoPalabra::SUST) {
            b.block = nuevaPalabra;
            break;
        }
    }
}

std::vector<TipoPalabra> Oracion::getSecuenciaTipos() const {
    std::vector<TipoPalabra> res;
    res.reserve(bloques_.size());
    for (const auto& b : bloques_) res.push_back(b.typo_b);
    return res;
}

void Oracion::save() {
    SentenceRepository::save(*this);
}

Oracion Oracion::loadById(int id) {
    return SentenceRepository::loadById(id);
}

Oracion Oracion::loadByKey(const std::string& clave, TipoPalabra tipo) {
    return SentenceRepository::loadByKey(clave, tipo);
}

void SentenceRepository::save(Oracion& oracion) {
    sqlite3* db = Database::instance().getHandle();
    if (!db) return;

    sqlite3_stmt* stmt;
    int newId = oracion.getId();

    if (newId > 0) {
        // UPDATE oraciones
        const char* sql_upd = "UPDATE oraciones SET clave_palabra=?, tipo_clave=?, frecuencia=?, tiempo_oracion=?, num_bloques=? WHERE id=?";
        if(!prepareStatement(db, sql_upd, &stmt))return;
        sqlite3_bind_text(stmt, 1, oracion.getClave().block.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, static_cast<int>(oracion.getClave().typo_b));
        sqlite3_bind_double(stmt, 3, oracion.getFrecuencia());
        sqlite3_bind_int(stmt, 4, static_cast<int>(oracion.getTiempo()));
        sqlite3_bind_int(stmt, 5, oracion.getNumBloques());
        sqlite3_bind_int(stmt, 6, newId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        // Eliminar bloques antiguos
        const char* sql_del = "DELETE FROM bloques WHERE oracion_id=?";
        if(!prepareStatement(db, sql_del, &stmt))return;
        sqlite3_bind_int(stmt, 1, newId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        // INSERT nueva oración
        const char* sql_ins = "INSERT INTO oraciones (clave_palabra, tipo_clave, frecuencia, tiempo_oracion, num_bloques) VALUES (?,?,?,?,?)";
        if(!prepareStatement(db, sql_ins, &stmt))return;
        sqlite3_bind_text(stmt, 1, oracion.getClave().block.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, static_cast<int>(oracion.getClave().typo_b));
        sqlite3_bind_double(stmt, 3, oracion.getFrecuencia());
        sqlite3_bind_int(stmt, 4, static_cast<int>(oracion.getTiempo()));
        sqlite3_bind_int(stmt, 5, oracion.getNumBloques());
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Error insertando oración: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return;
        }
        newId = static_cast<int>(sqlite3_last_insert_rowid(db));
        sqlite3_finalize(stmt);
        // Asignar el nuevo ID a la oración
        oracion.setId(newId);
    }

    // Insertar los bloques
    const char* sql_bloque = "INSERT INTO bloques (oracion_id, posicion, bloque_texto, tipo_palabra) VALUES (?,?,?,?)";
    if(!prepareStatement(db, sql_bloque, &stmt))return;
    const auto& bloques = oracion.getBloques();
    for (size_t i = 0; i < bloques.size(); ++i) {
        sqlite3_bind_int(stmt, 1, newId);
        sqlite3_bind_int(stmt, 2, static_cast<int>(i));
        sqlite3_bind_text(stmt, 3, bloques[i].block.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, static_cast<int>(bloques[i].typo_b));
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Error insertando bloque: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);
}

Oracion SentenceRepository::loadById(int id) {
    sqlite3* db = Database::instance().getHandle();
    Oracion oracion;
    if (!db) return oracion;

    // Obtener datos de la tabla oraciones
    sqlite3_stmt* stmt;
    const char* sql_oracion = "SELECT clave_palabra, tipo_clave, frecuencia, tiempo_oracion FROM oraciones WHERE id = ?";
    if (!prepareStatement(db, sql_oracion, &stmt)) return oracion;
    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return oracion;
    }

    std::string clave_palabra = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    TipoPalabra tipo_clave = static_cast<TipoPalabra>(sqlite3_column_int(stmt, 1));
    float frecuencia = static_cast<float>(sqlite3_column_double(stmt, 2));
    Tiempo tiempo_oracion = static_cast<Tiempo>(sqlite3_column_int(stmt, 3));
    sqlite3_finalize(stmt);

    // Cargar bloques
    const char* sql_bloques = "SELECT bloque_texto, tipo_palabra FROM bloques WHERE oracion_id = ? ORDER BY posicion";
    if (!prepareStatement(db, sql_bloques, &stmt)) return oracion;
    sqlite3_bind_int(stmt, 1, id);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* texto = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        TipoPalabra tipo = static_cast<TipoPalabra>(sqlite3_column_int(stmt, 1));
        oracion.addBloque(texto, tipo);
    }
    sqlite3_finalize(stmt);

    // Asignar propiedades
    oracion.setId(id);
    oracion.setClave({clave_palabra, tipo_clave});
    oracion.setFrecuencia(frecuencia);
    oracion.setTiempo(tiempo_oracion);

    return oracion;
}

Oracion SentenceRepository::loadByKey(const std::string& clave, TipoPalabra tipo) {
    sqlite3* db = Database::instance().getHandle();
    Oracion oracion;
    if (!db) return oracion;

    sqlite3_stmt* stmt;
    const char* sql = "SELECT id FROM oraciones WHERE clave_palabra = ? AND tipo_clave = ? LIMIT 1";
    if (!prepareStatement(db, sql, &stmt)) return oracion;
    sqlite3_bind_text(stmt, 1, clave.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, static_cast<int>(tipo));
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return oracion;
    }
    int id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return loadById(id);
}
