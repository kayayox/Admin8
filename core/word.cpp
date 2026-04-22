#include "word.hpp"
#include "../db/word_repository.hpp"
#include "../utils/string_conv.hpp"
#include <cstring>

Word::Word(const std::string& palabra) : palabra_(palabra) {
    load(); // intenta cargar desde BD
}

void Word::setSignificado(const std::string& sig, bool save_to_db) {
    significado_ = sig;
    if (save_to_db) save();
}

void Word::setTipo(TipoPalabra tipo, bool save_to_db) {
    tipo_ = tipo;
    if (save_to_db) save();
}

void Word::setCantidad(Cantidad cant, bool save_to_db) {
    cantidad_ = cant;
    if (save_to_db) save();
}

void Word::setTiempo(Tiempo tiempo, bool save_to_db) {
    tiempo_ = tiempo;
    if (save_to_db) save();
}

void Word::setGenero(Genero gen, bool save_to_db) {
    genero_ = gen;
    if (save_to_db) save();
}

void Word::setGrado(Grado grado, bool save_to_db) {
    grado_ = grado;
    if (save_to_db) save();
}

void Word::setPersona(Persona pers, bool save_to_db) {
    persona_ = pers;
    if (save_to_db) save();
}

void Word::setConfianza(float conf, bool save_to_db) {
    confianza_ = conf;
    if (save_to_db) save();
}

void Word::generateDefaultMeaning(const std::string& contexto) {
    std::string tipo_str = tipoToString(tipo_);
    if (!contexto.empty()) {
        significado_ = tipo_str + ": '" + palabra_ + "' (detectado automáticamente en: \"" + contexto + "\")";
    } else {
        significado_ = tipo_str + ": '" + palabra_ + "' (clasificación automática)";
    }
    save();
}

void Word::save() const {
    WordRepository::save(*this);
}

bool Word::load() {
    Word w;
    if (WordRepository::load(palabra_, w)) {
        *this = std::move(w);
        return true;
    }
    return false;
}

void Word::addRelated(const std::string& palabra, double valor) {
    relacionadas_.emplace_back(palabra, valor);
    save();
}
