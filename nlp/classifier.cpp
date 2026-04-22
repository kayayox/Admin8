#include "classifier.hpp"
#include "hmm_tagger.hpp"
#include "morphology.hpp"
#include "context_memory.hpp"
#include "../utils/string_conv.hpp"
#include "../db/word_repository.hpp"
#include <iostream>
#include <cctype>

Classifier::Classifier(CorpusStats& stats) : stats_(stats) {}

void Classifier::classifySentence(std::vector<Word>& words, const ContextInfo& global_ctx) {
    if (words.empty()) return;

    // Extraer tokens
    std::vector<std::string> tokens;
    tokens.reserve(words.size());
    for (const auto& w : words) tokens.push_back(w.getPalabra());

    // Obtener etiquetas con probabilidad posterior
    auto tagged = hmm::tagWithPosteriorProb(tokens, stats_);

    // Recorrer cada palabra
    for (size_t i = 0; i < words.size(); ++i) {
        Word& w = words[i];
        TipoPalabra tag_hmm = tagged[i].first;
        float conf_hmm = static_cast<float>(tagged[i].second);

        //  Construir ContextInfo específico para esta palabra
        ContextInfo ctx;
        ctx.palabra_anterior = (i > 0) ? words[i-1].getPalabra() : "";
        ctx.palabra_siguiente = (i+1 < words.size()) ? words[i+1].getPalabra() : "";
        ctx.es_primera_de_oracion = (i == 0);
        ctx.tiene_mayuscula_inicial = !w.getPalabra().empty() && std::isupper(w.getPalabra()[0]);
        ctx.puntuacion_anterior = 0; // se podria obtener del tokenizador->pendiente
        ctx.texto_previo = global_ctx.texto_previo; // conservar contexto global si existe
        // Opcional: etiqueta anterior si ya tenemos clasificada la palabra anterior
        if (i > 0) ctx.etiqueta_anterior = words[i-1].getTipo();

        // Consultar memoria contextual
        auto mem = ContextMemory::query(w.getPalabra(), ctx);
        TipoPalabra tag_final = tag_hmm;
        float conf_final = conf_hmm;
        bool used_memory = false;

        if (mem && mem->confidence >= 0.85f) {
            // Memoria fuerte: usar su etiqueta
            tag_final = mem->tag;
            conf_final = mem->confidence;
            used_memory = true;
            // Reforzar automaticamente
            ContextMemory::reinforce(w.getPalabra(), ctx, tag_final, conf_final);
        } else if (mem && mem->confidence >= 0.6f) {
            // Memoria moderada: comparar con HMM
            if (tag_hmm == mem->tag) {
                // Coinciden: reforzar
                ContextMemory::reinforce(w.getPalabra(), ctx, tag_hmm, conf_hmm);
                conf_final = (conf_hmm + mem->confidence) / 2.0f;
            } else {
                // Discrepan: reducir confianza
                conf_final = conf_hmm * 0.5f;
                // No usamos la etiqueta de memoria, mantenemos la del HMM
            }
        }

        // Aplicar regla específica para pronombres
        if (tag_final == TipoPalabra::PRON) {
            tag_final = morphology::guessInitialTag(w.getPalabra());
        }

        // Si la palabra ya tenía tipo definido cargado desde BD, ajustar confianza
        if (!w.load()) {
            TipoPalabra old_tipo = w.getTipo();
            if (old_tipo != TipoPalabra::INDEFINIDO) {
                if (tag_final == old_tipo) conf_final = 0.7f;
                else conf_final = 0.4f;
            } else {
                w.setTipo(tag_final, false);
            }
            w.setConfianza(conf_final, false);

            // Rellenar atributos morfológicos
            const std::string& pal = w.getPalabra();
            switch (w.getTipo()) {
                case TipoPalabra::VERB:
                    w.setTiempo(morphology::detectTense(pal));
                    w.setPersona(morphology::detectPerson(pal));
                    w.setCantidad(morphology::endsWith(pal, "n") || morphology::endsWith(pal, "mos") ? Cantidad::PLUR : Cantidad::SING);
                    break;
                case TipoPalabra::SUST:
                case TipoPalabra::ADJT:
                    w.setCantidad(morphology::isPlural(pal) ? Cantidad::PLUR : Cantidad::SING);
                    w.setGenero(morphology::detectGender(pal));
                    if (w.getTipo() == TipoPalabra::ADJT)
                        w.setGrado(morphology::detectAdjectiveDegree(pal));
                    break;
                case TipoPalabra::ART:
                    w.setCantidad(morphology::isPlural(pal) ? Cantidad::PLUR : Cantidad::SING);
                    break;
                default:
                    break;
            }

            w.generateDefaultMeaning(ctx.texto_previo);
        }

        // Feedback solo si confianza baja y no se uso memoria fuerte
        if (conf_final < 0.5f && !used_memory) {
            requestCorrection(w, ctx);
        }

        w.save();
    }
}

void Classifier::classifyWord(Word& w, const ContextInfo& ctx) {
    std::vector<Word> words = {w};
    classifySentence(words, ctx);
    w = words[0]; // actualizar
}

void Classifier::requestCorrection(Word& w, const ContextInfo& ctx) {
    std::cout << "\n[El sistema no está seguro de la palabra '" << w.getPalabra() << "']\n";
    std::cout << "Clasificación actual: " << tipoToString(w.getTipo()) << " (confianza " << (w.getConfianza() * 100) << "%)\n";
    std::cout << "¿Es correcta? (s/n): ";
    std::string resp;
    std::getline(std::cin, resp);
    if (resp.empty()) return;

    if (resp[0] == 's' || resp[0] == 'S') {
        updateConfidence(w, true);
        std::cout << "¡Gracias! La confianza ha aumentado.\n";
        stats_.updateFromCorrection(w.getPalabra(), w.getTipo(), "", TipoPalabra::INDEFINIDO);
        // Reforzar en memoria contextual
        ContextMemory::reinforce(w.getPalabra(), ctx, w.getTipo(), w.getConfianza());
    } else {
        std::cout << "Indique el tipo correcto:\n";
        std::cout << "1. Sustantivo  2. Verbo  3. Adjetivo  4. Adverbio  5. Preposición\n";
        std::cout << "6. Conjunción  7. Artículo  8. Pronombre  9. Otro\n";
        std::string opt;
        std::getline(std::cin, opt);
        int op = std::stoi(opt);
        TipoPalabra nuevo_tipo = TipoPalabra::INDEFINIDO;
        switch (op) {
            case 1: nuevo_tipo = TipoPalabra::SUST; break;
            case 2: nuevo_tipo = TipoPalabra::VERB; break;
            case 3: nuevo_tipo = TipoPalabra::ADJT; break;
            case 4: nuevo_tipo = TipoPalabra::ADV; break;
            case 5: nuevo_tipo = TipoPalabra::PREP; break;
            case 6: nuevo_tipo = TipoPalabra::CONJ; break;
            case 7: nuevo_tipo = TipoPalabra::ART; break;
            case 8: nuevo_tipo = TipoPalabra::PRON; break;
            default: break;
        }
        if (nuevo_tipo != TipoPalabra::INDEFINIDO) {
            TipoPalabra old_tag = w.getTipo();
            w.setTipo(nuevo_tipo, false);
            updateConfidence(w, false);
            w.setSignificado("Corregido por usuario a " + tipoToString(nuevo_tipo));
            w.save();
            std::cout << "Corrección guardada. Aprendiendo...\n";
            stats_.updateFromCorrection(w.getPalabra(), nuevo_tipo, "", old_tag);
            // Guardar en memoria contextual con alta confianza
            ContextMemory::recordCorrection(w.getPalabra(), ctx, nuevo_tipo, w.getConfianza());
        }
        w.save();
    }
}

void Classifier::updateConfidence(Word& w, bool acierto) {
    float conf = w.getConfianza();
    if (acierto) {
        conf += (1.0f - conf) * 0.3f;
    } else {
        conf += 0.35f;
    }
    if (conf > 0.99f) conf = 0.99f;
    if (conf < 0.1f) conf = 0.1f;
    w.setConfianza(conf,true);
}
