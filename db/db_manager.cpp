#include "db_manager.hpp"


Database& Database::instance() {
    static Database inst;
    return inst;
}

bool Database::init(const std::string& path) {
    int rc = sqlite3_open(path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Error abriendo BD: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    const char* sql =
        "CREATE TABLE IF NOT EXISTS palabras ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "palabra TEXT UNIQUE NOT NULL,"
        "significado TEXT,"
        "tipo INTEGER,"
        "cantidad INTEGER,"
        "tiempo INTEGER,"
        "genero INTEGER,"
        "grado INTEGER,"
        "persona INTEGER,"
        "num_relacionadas INTEGER DEFAULT 0,"
        "confianza REAL"
        ");"
        "CREATE TABLE IF NOT EXISTS relaciones ("
        "palabra_id INTEGER, relacionada_id INTEGER, valor REAL,"
        "FOREIGN KEY(palabra_id) REFERENCES palabras(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS oraciones ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "clave_palabra TEXT, tipo_clave INTEGER, frecuencia REAL,"
        "tiempo_oracion INTEGER, num_bloques INTEGER"
        ");"
        "CREATE TABLE IF NOT EXISTS bloques ("
        "oracion_id INTEGER, posicion INTEGER, bloque_texto TEXT, tipo_palabra INTEGER,"
        "FOREIGN KEY(oracion_id) REFERENCES oraciones(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS patrones ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "tipo_patron INTEGER, secuencia TEXT, frecuencia REAL DEFAULT 1.0"
        ");"
        "CREATE TABLE IF NOT EXISTS dialogos ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "premisa_id INTEGER, hipotesis_id INTEGER, tipo_patron INTEGER,"
        "creatividad REAL, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");"
        "CREATE TABLE IF NOT EXISTS feedback ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "palabra TEXT, tipo_propuesto INTEGER, tipo_correcto INTEGER,"
        "acierto INTEGER, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");"
        "CREATE TABLE IF NOT EXISTS context_memory ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "palabra TEXT NOT NULL,"
        "context_hash TEXT NOT NULL,"
        "tag INTEGER NOT NULL,"
        "confidence REAL NOT NULL,"
        "frequency INTEGER NOT NULL DEFAULT 1,"
        "last_seen DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "UNIQUE(palabra, context_hash)"
        ");";
    char* errmsg = nullptr;
    rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Error creando tablas: " << errmsg << std::endl;
        sqlite3_free(errmsg);
        return false;
    }
    return true;
}

void Database::close() {
    if (db_) {
        int rc = sqlite3_close(db_);
        if (rc != SQLITE_OK) {
            std::cerr << "Error al cerrar BD: " << sqlite3_errmsg(db_)
                      << " (código " << rc << ")" << std::endl;
        }
        db_ = nullptr;
    }
}
