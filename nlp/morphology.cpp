/*Archivo aun en desarrollo espero realizar una modificacion mayor para que los terminos sean
cargados desde Archivos binarios donde puedan crecer a medida que el modelo aprende*/
#include "morphology.hpp"
#include <vector>
#include <algorithm>
#include <cctype>

namespace morphology {

    static const std::vector<std::string> sufijos_sustantivo = {
        "cion", "dad", "tad", "sión", "aje", "ente", "ante", "or", "ora", "umbre", "ez", "eza", "ista", "ismo", "tud", "ambre"
    };
    static const std::vector<std::string> sufijos_verbo = {
        "ar", "er", "ir", "ando", "iendo", "aba", "ia", "are", "ere", "ire", "aria", "eria", "iria",
        "aste", "iste", "io", "aron", "ieron", "ado", "ido"
    };
    static const std::vector<std::string> sufijos_adjetivo = {
        "oso", "osa", "ble", "ivo", "iva", "al", "il", "ble", "ante", "ista", "ario", "aria", "ero", "era", "dor", "dora", "nte"
    };
    static const std::vector<std::string> sufijos_adverbio = { "mente" };
    static const std::vector<std::string> sufijos_conjugados = {
        "o", "as", "a", "amos", "áis", "an", "o", "es", "e", "emos", "éis", "en",
        "o", "es", "e", "imos", "ís", "en", "é", "aste", "ó", "amos", "asteis", "aron",
        "i", "iste", "io", "imos", "isteis", "ieron", "aba", "abas", "aba", "ábamos", "abais", "aban",
        "ia", "ias", "ia", "iamos", "iais", "ian", "are", "aras", "ara", "aremos", "areis", "aran",
        "ere", "eras", "era", "eremos", "ereis", "eran", "ire", "iras", "irá", "iremos", "ireis", "iran",
        "aria", "arias","ariamos", "ariais", "arian", "eria", "erias", "eria", "eriamos", "eriais", "erian",
        "iria", "irias", "iria", "iriamos", "iriais", "irian"
    };
    static const std::vector<std::string> comunes_sustantivos = {
        "casa", "perro", "gato", "hombre", "mujer", "niño", "niña", "padre", "madre", "hermano",
        "hermana", "amigo", "amiga", "trabajo", "escuela", "ciudad", "país", "mundo", "vida", "día",
        "noche", "tiempo", "año", "mes", "semana", "hora", "minuto", "persona", "familia", "empresa",
        "producto", "servicio", "problema", "solución", "idea", "coche", "moto", "avion", "barco", "tren"
    };
    static const std::vector<std::string> comunes_adjetivos = {
        "bueno", "buena", "malo", "mala", "grande", "pequeño", "pequeña", "nuevo", "nueva", "viejo",
        "vieja", "joven", "alto", "alta", "bajo", "baja", "largo", "larga", "corto", "corta",
        "facil", "dificil", "rapido", "rapida", "lento", "lenta", "caliente", "frio", "fria", "feliz"
    };
    static const std::vector<std::string> irregulares = {
        "ser", "ir", "haber", "estar", "tener", "hacer", "poder", "decir", "ver", "dar",
        "saber", "querer", "llegar", "pasar", "deber", "poner", "parecer", "quedar", "creer",
        "hablar", "llevar", "dejar", "seguir", "encontrar", "llamar", "venir", "pensar", "salir",
        "volver", "tomar", "conocer", "vivir", "sentir", "tratar", "mirar", "contar", "empezar"
    };
    static const std::vector<std::string> preposiciones = {"a", "ante", "bajo", "con", "de", "en", "para", "por", "sin", "sobre"};
    static const std::vector<std::string> conjunciones = {"y", "o", "pero", "porque", "si", "aunque"};
    static const std::vector<std::string> articulos = {"el", "la", "los", "las", "un", "una", "unos", "unas"};

    bool endsWith(const std::string& palabra, const std::string& sufijo) {
        if (sufijo.size() > palabra.size()) return false;
        return palabra.compare(palabra.size() - sufijo.size(), sufijo.size(), sufijo) == 0;
    }

    static bool endsWithAny(const std::string& palabra, const std::vector<std::string>& lista) {
        for (const auto& suf : lista)
            if (endsWith(palabra, suf)) return true;
        return false;
    }

    static bool isInList(const std::string& palabra, const std::vector<std::string>& lista) {
        return std::find(lista.begin(), lista.end(), palabra) != lista.end();
    }

    bool isPlural(const std::string& palabra) {
        static const std::vector<std::string> excepciones = {"crisis", "martes", "paraguas", "lunes", "miércoles", "jueves", "viernes", "sábado", "domingo", "tórax", "fórceps", "virus", "atlas", "mes", "país"};
        if (isInList(palabra, excepciones)) return false;
        if (palabra.size() < 2) return false;
        return palabra.back() == 's';
    }

    Genero detectGender(const std::string& palabra) {
        static const std::vector<std::string> ex_fem = {"mapa", "día", "problema", "sistema", "idioma", "clima", "programa", "tema", "poema", "esquema"};
        static const std::vector<std::string> ex_masc = {"mano", "radio", "moto", "foto"};
        if (isInList(palabra, ex_fem)) return Genero::MASC;
        if (isInList(palabra, ex_masc)) return Genero::FEME;

        static const std::vector<std::string> term_fem = {"a", "ción", "sión", "dad", "tad", "umbre", "eza", "iz", "triz"};
        static const std::vector<std::string> term_masc = {"o", "l", "n", "r", "s", "ma", "ta", "pa", "aje"};
        if (endsWithAny(palabra, term_fem)) return Genero::FEME;
        if (endsWithAny(palabra, term_masc)) return Genero::MASC;
        return Genero::NEUT;
    }

    Tiempo detectTense(const std::string& palabra) {
        if (endsWith(palabra, "ó") || endsWith(palabra, "ió") || endsWith(palabra, "aba") ||
            endsWith(palabra, "ía") || endsWith(palabra, "aste") || endsWith(palabra, "iste") ||
            endsWith(palabra, "aron") || endsWith(palabra, "ieron") || endsWith(palabra, "ado") ||
            endsWith(palabra, "ido"))
            return Tiempo::PASS;
        if (endsWith(palabra, "aré") || endsWith(palabra, "eré") || endsWith(palabra, "iré") ||
            endsWith(palabra, "ará") || endsWith(palabra, "erá") || endsWith(palabra, "irá"))
            return Tiempo::FUTR;
        return Tiempo::PRES;
    }

    Persona detectPerson(const std::string& palabra) {
        if (endsWith(palabra, "o") && !endsWith(palabra, "mos") && !endsWith(palabra, "ís") && !endsWith(palabra, "n"))
            return Persona::PRIM;
        if (endsWith(palabra, "as") || endsWith(palabra, "es") || endsWith(palabra, "ís"))
            return Persona::SEGU;
        if (endsWith(palabra, "a") || endsWith(palabra, "e"))
            return Persona::TERC;
        if (endsWith(palabra, "mos"))
            return Persona::PRIM;
        if (endsWith(palabra, "n"))
            return Persona::TERC;
        return Persona::NIN;
    }

    Grado detectAdjectiveDegree(const std::string& palabra) {
        if (endsWith(palabra, "ísimo") || endsWith(palabra, "érrimo") ||
            endsWith(palabra, "ísima") || endsWith(palabra, "érrima"))
            return Grado::SUPERLA;
        return Grado::POSIT;
    }

    TipoPalabra guessInitialTag(const std::string& palabra) {
        // Verbos
        if (endsWithAny(palabra, sufijos_verbo) || isInList(palabra, irregulares))
            return TipoPalabra::VERB;
        // Sustantivos
        if (endsWithAny(palabra, sufijos_sustantivo) || isInList(palabra, comunes_sustantivos))
            return TipoPalabra::SUST;
        // Adjetivos
        if (endsWithAny(palabra, sufijos_adjetivo) || isInList(palabra, comunes_adjetivos))
            return TipoPalabra::ADJT;
        // Adverbios
        if (endsWithAny(palabra, sufijos_adverbio))
            return TipoPalabra::ADV;
        // Funcionales
        if (isInList(palabra, preposiciones)) return TipoPalabra::PREP;
        if (isInList(palabra, conjunciones)) return TipoPalabra::CONJ;
        if (isInList(palabra, articulos)) return TipoPalabra::ART;
        return TipoPalabra::INDEFINIDO;
    }
    /// Valores arbitrarios aun en revision, es posible que sean luego mejorados por estadistica
    float getSuffixProb(const std::string& palabra, TipoPalabra tag) {
        float base = (palabra.size() < 3) ? 0.01f : 0.05f;
        switch (tag) {
            case TipoPalabra::SUST:
                if (endsWithAny(palabra, sufijos_sustantivo)) return 0.85f;
                break;
            case TipoPalabra::VERB:
                if (endsWithAny(palabra, sufijos_verbo)) return 0.90f;
                if (endsWithAny(palabra, sufijos_conjugados)) return 0.80f;
                break;
            case TipoPalabra::ADJT:
                if (endsWithAny(palabra, sufijos_adjetivo)) return 0.75f;
                break;
            case TipoPalabra::ADV:
                if (endsWithAny(palabra, sufijos_adverbio)) return 0.70f;
                break;
            case TipoPalabra::ART:
                if (isInList(palabra, articulos)) return 0.99f;
                return 0.05f;
            case TipoPalabra::PREP:
                if (isInList(palabra, preposiciones)) return 0.98f;
                return 0.05f;
            case TipoPalabra::CONJ:
                if (isInList(palabra, conjunciones)) return 0.97f;
                return 0.05f;
            default:
                return 0.001f;
        }
        return base;
    }

} // namespace morphology
