/** Prototipo de Api para pruebas,es posible
que se tengan varias para propositos distintos*/
#include "nlp_api.h"
#include "core/word.hpp"
#include "dialogue/PatternCorrelator.h"
#include "core/ContextualCorrelator.h"
#include "nlp/classifier.hpp"
#include "nlp/corpus_stats.hpp"
#include "nlp/morphology.hpp"
#include "nlp/context_memory.hpp"
#include "nlp/context_info.hpp"
#include "db/db_manager.hpp"
#include "db/word_repository.hpp"
#include "db/sentence_repository.hpp"
#include "db/dialogue_repository.hpp"
#include "utils/string_conv.hpp"
#include "tokenizer/tokenizer.hpp"
#include "core/Learning.h"
#include <sstream>
#include <iostream>
#include <memory>

class NLPEngine::Impl {
public:
    std::string semanticDbPath;
    std::string correlatorDbPath;
    std::unique_ptr<PatternCorrelator> patternCorr;
    std::unique_ptr<ContextualCorrelator> ctxCorr;
    std::unique_ptr<CorpusStats> stats;
    std::unique_ptr<Classifier> classifier;
    bool initialized = false;

    bool initDB(const std::string& semPath, const std::string& corrPath) {
        semanticDbPath = semPath;
        correlatorDbPath = corrPath;
        if (!Database::instance().init(semanticDbPath)) {
            std::cerr << "[ERROR] No se pudo abrir BD semántica: " << semanticDbPath << std::endl;
            return false;
        }

        // Cargar estadisticas iniciales
        stats = std::make_unique<CorpusStats>();
        auto corpus = DialogueRepository::buildCorpus();
        for (const auto& word : corpus) {
            stats->updateFromCorrection(word, morphology::guessInitialTag(word), "", TipoPalabra::INDEFINIDO);
        }
        classifier = std::make_unique<Classifier>(*stats);
        try {
            patternCorr = std::make_unique<PatternCorrelator>(correlatorDbPath);
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] PatternCorrelator: " << e.what() << std::endl;
            return false;
        }
        ctxCorr = std::make_unique<ContextualCorrelator>(*patternCorr);

        initialized = true;
        return true;
    }

    void closeDB() {
        if (initialized) {
            // Liberar recursos internos antes de cerrar la BD
            patternCorr.reset();
            ctxCorr.reset();
            classifier.reset();
            stats.reset();
            // Cerrar la conexión singleton de Database
            Database::instance().close();
            initialized = false;
        }
    }

    void learnText(const std::string& text) {
        if (!initialized || text.empty()) {
            std::cerr << "[ERROR] learnText llamado sin inicializar" << std::endl;
            return;
        }
        ::learnText(*ctxCorr, *patternCorr, text);

        // Tokenizar y clasificar palabras nuevas
        auto tokens = tokenize(text);
        std::vector<Word> words;
        for (const auto& tok : tokens) {
            if(tok.type==TokenType::WORD){
                Word w(tok.text);
                if (w.getTipo() == TipoPalabra::INDEFINIDO) {
                    ContextInfo ctx;
                    classifier->classifyWord(w, ctx);   // No hay interacción con consola
                }
                words.push_back(w);
            }else{
                Word x(tok.text);
                x.setTipo((tok.type==TokenType::DATE)? TipoPalabra::DATE : TipoPalabra::NUM);
                words.push_back(x);
            }
        }
        if (!words.empty()) {
            Oracion sent(words);
            sent.save();
        }
    }

    std::vector<WordInfo> classifySentence(const std::string& sentence) {
        auto tokens = tokenize(sentence);
        std::vector<Word> words;
        ContextInfo ctx;
        ctx.texto_previo = sentence;
        for (const auto& tok : tokens) {
            Word w(tok.text);
            if (w.getTipo() == TipoPalabra::INDEFINIDO) {
                classifier->classifyWord(w, ctx);
            }
            words.push_back(w);
        }
        std::vector<WordInfo> result;
        for (const auto& w : words) {
            WordInfo info;
            info.word = w.getPalabra();
            info.tipo = tipoToString(w.getTipo());
            info.confianza = w.getConfianza();
            info.significado = w.getSignificado();
            info.cantidad = cantidadToString(w.getCantidad());
            info.tiempo = tiempoToString(w.getTiempo());
            info.genero = generoToString(w.getGenero());
            info.persona = personaToString(w.getPersona());
            info.grado = gradoToString(w.getGrado());
            result.push_back(info);
        }
        return result;
    }

    WordInfo classifyWord(const std::string& word, const std::string& context) {
        Word w(word);
        ContextInfo ctx;
        ctx.texto_previo = context;
        classifier->classifyWord(w, ctx);
        WordInfo info;
        info.word = w.getPalabra();
        info.tipo = tipoToString(w.getTipo());
        info.confianza = w.getConfianza();
        info.significado = w.getSignificado();
        info.cantidad = cantidadToString(w.getCantidad());
        info.tiempo = tiempoToString(w.getTiempo());
        info.genero = generoToString(w.getGenero());
        info.persona = personaToString(w.getPersona());
        info.grado = gradoToString(w.getGrado());
        return info;
    }

    std::vector<Prediction> predictNextWords(const std::string& currentWord,
                                             const std::vector<std::string>& previousWords) {
        if (!initialized) return {};
        std::vector<std::pair<Pattern, double>> outcomes;
        if (!ctxCorr->queryNext(currentWord, previousWords, outcomes)) {
            return {};
        }
        std::vector<Prediction> preds;
        for (const auto& p : outcomes) {
            if (!p.first.empty()) {
                Prediction pred;
                pred.word = p.first.begin()->first;
                pred.probability = p.second;
                preds.push_back(pred);
            }
        }
        return preds;
    }

    std::string generateHypothesis(const std::string& premise) {
        auto tokens = tokenize(premise);
        std::vector<Word> words;
        for (const auto& tok : tokens) {
            words.emplace_back(tok.text);
        }
        Oracion prem(words);
        Patron pat = patronFromSecuencia(prem.getSecuenciaTipos());
        DialogueHistory hist = DialogueRepository::loadHistory();
        const Patron* usedPat = &pat;
        for (const auto& d : hist.getHistory()) {
            if (d.patron.tipo == pat.tipo) {
                usedPat = &d.patron;
                break;
            }
        }
        Oracion hip = ::generateHypothesis(prem, usedPat, "");
        std::stringstream ss;
        for (const auto& blk : hip.getBloques())
            ss << blk.block << " ";
        std::string result = ss.str();
        return result;
    }
};

// Implementación de la fachada pública

NLPEngine::NLPEngine() : pImpl(std::make_unique<Impl>()) {}
//NLPEngine::~NLPEngine() = default;
NLPEngine::~NLPEngine() {
    pImpl->closeDB();
}

bool NLPEngine::initialize(const std::string& semanticDbPath, const std::string& correlatorDbPath) {
    return pImpl->initDB(semanticDbPath, correlatorDbPath);
}

void NLPEngine::shutdown() {
    pImpl->closeDB();
}

void NLPEngine::learnText(const std::string& text) {
    pImpl->learnText(text);
}

std::vector<WordInfo> NLPEngine::classifySentence(const std::string& sentence) {
    return pImpl->classifySentence(sentence);
}

WordInfo NLPEngine::classifyWord(const std::string& word, const std::string& context) {
    return pImpl->classifyWord(word, context);
}

std::vector<Prediction> NLPEngine::predictNextWords(const std::string& currentWord,
                                                    const std::vector<std::string>& previousWords) {
    return pImpl->predictNextWords(currentWord, previousWords);
}

std::vector<Prediction> NLPEngine::predictNextWordWithOnePrev(const std::string& currentWord,
                                                              const std::string& previousWord) {
    return predictNextWords(currentWord, {previousWord});
}

std::vector<Prediction> NLPEngine::predictNextWordWithTwoPrev(const std::string& currentWord,
                                                              const std::string& prev1,
                                                              const std::string& prev2) {
    return predictNextWords(currentWord, {prev2, prev1});
}

std::string NLPEngine::generateHypothesis(const std::string& premiseSentence) {
    return pImpl->generateHypothesis(premiseSentence);
}

WordInfo NLPEngine::getWordInfo(const std::string& word) {
    return classifyWord(word);
}

void NLPEngine::correctWord(const std::string& word, const std::string& correctType) {
    Word w(word);
    TipoPalabra newType = TipoPalabra::INDEFINIDO;
    if (correctType == "Sustantivo") newType = TipoPalabra::SUST;
    else if (correctType == "Verbo") newType = TipoPalabra::VERB;
    else if (correctType == "Adjetivo") newType = TipoPalabra::ADJT;
    else if (correctType == "Adverbio") newType = TipoPalabra::ADV;
    else if (correctType == "Preposición") newType = TipoPalabra::PREP;
    else if (correctType == "Conjunción") newType = TipoPalabra::CONJ;
    else if (correctType == "Artículo") newType = TipoPalabra::ART;
    else if (correctType == "Pronombre") newType = TipoPalabra::PRON;

    if (newType != TipoPalabra::INDEFINIDO) {
        w.setTipo(newType);
        w.setConfianza(0.95f);
        w.setSignificado("Corregido por API a " + correctType);
        w.save();

        // Actualizar estadisticas del clasificador
        pImpl->stats->updateFromCorrection(word, newType, "", w.getTipo());

        // Actualizar memoria contextual con un contexto generico
        ContextInfo ctx;
        ctx.texto_previo = "corrección manual";
        ContextMemory::recordCorrection(word, ctx, newType, 0.95f);
    }
}
