#ifndef SENTENCE_REPOSITORY_HPP
#define SENTENCE_REPOSITORY_HPP

#include "../types.hpp"
#include "../core/word.hpp"
#include <string>
#include <vector>

struct Bloque {
    std::string block="";
    TipoPalabra typo_b= TipoPalabra::INDEFINIDO;
};

class Oracion {
public:
    Oracion() = default;
    explicit Oracion(std::vector<Word>& words); // desde palabras

    // Getters
    int getId() const { return id_; }
    void setId(int id){this->id_=id;}
    void setClave(const Bloque& c) { clave_ = c; }
    void setFrecuencia(float f) { frecuencia_ = f; }
    void setTiempo(Tiempo t) { tiempo_ = t; }
    const std::vector<Bloque>& getBloques() const { return bloques_; }
    Tiempo getTiempo() const { return tiempo_; }
    const Bloque& getClave() const { return clave_; }
    float getFrecuencia() const { return frecuencia_; }
    int getNumBloques() const { return static_cast<int>(bloques_.size()); }

    // Modificadores para generacion de hipotesis
    void addBloque(const std::string& texto, TipoPalabra tipo);
    void insertarBloqueInicio(const std::string& texto, TipoPalabra tipo);
    void insertarNegacion();
    void reemplazarSustantivo(const std::string& nuevaPalabra);
    std::vector<TipoPalabra> getSecuenciaTipos() const;

    // Persistencia
    void save();
    static Oracion loadById(int id);
    static Oracion loadByKey(const std::string& clave, TipoPalabra tipo);

private:
    int id_ = -1;
    std::vector<Bloque> bloques_;
    Tiempo tiempo_ = Tiempo::INDETERMINADO;
    Bloque clave_;
    float frecuencia_ = 1.0f;
};

class SentenceRepository {
public:
    static void save(Oracion& oracion);
    static Oracion loadById(int id);
    static Oracion loadByKey(const std::string& clave, TipoPalabra tipo);
};

#endif // SENTENCE_REPOSITORY_HPP
