#ifndef WORD_HPP
#define WORD_HPP

#include "../types.hpp"
#include <string>
#include <vector>

class Word {
public:
    // Constructores
    Word() = default;
    explicit Word(const std::string& palabra);
    Word(const Word&) = default;
    Word(Word&&) = default;
    Word& operator=(const Word&) = default;
    Word& operator=(Word&&) = default;

    // Getters
    const std::string& getPalabra() const { return palabra_; }
    const std::string& getSignificado() const { return significado_; }
    TipoPalabra getTipo() const { return tipo_; }
    Cantidad getCantidad() const { return cantidad_; }
    Tiempo getTiempo() const { return tiempo_; }
    Genero getGenero() const { return genero_; }
    Grado getGrado() const { return grado_; }
    Persona getPersona() const { return persona_; }
    float getConfianza() const { return confianza_; }

    // Setters
    void setPalabra(const std::string& p) { palabra_ = p; }
    void setSignificado(const std::string& sig, bool save_to_db = true);
    void setTipo(TipoPalabra tipo, bool save_to_db = true);
    void setCantidad(Cantidad cant, bool save_to_db = true);
    void setTiempo(Tiempo tiempo, bool save_to_db = true);
    void setGenero(Genero gen, bool save_to_db = true);
    void setGrado(Grado grado, bool save_to_db = true);
    void setPersona(Persona pers, bool save_to_db = true);
    void setConfianza(float conf, bool save_to_db = true);

    // Generar significado por defecto
    void generateDefaultMeaning(const std::string& contexto = "");

    // Persistencia
    void save() const;
    bool load(); // carga desde BD sobreescribe atributos

    // Relacionadas
    void addRelated(const std::string& palabra, double valor);
    const std::vector<std::pair<std::string, double>>& getRelated() const { return relacionadas_; }

private:
    std::string palabra_="";
    std::string significado_="";
    TipoPalabra tipo_ = TipoPalabra::INDEFINIDO;
    Cantidad cantidad_ = Cantidad::NONE;
    Tiempo tiempo_ = Tiempo::INDETERMINADO;
    Genero genero_ = Genero::NEUT;
    Grado grado_ = Grado::NON;
    Persona persona_ = Persona::NIN;
    float confianza_ = 0.0f;

    std::vector<std::pair<std::string, double>> relacionadas_;
    std::vector<std::string> variantes_;
};

#endif // WORD_HPP
