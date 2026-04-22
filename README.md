# Motor NLP – Procesamiento de Lenguaje Natural Contextual en C++

Un motor de procesamiento de lenguaje natural modular escrito en C++17. Realiza etiquetado morfosintáctico, predicción de la siguiente palabra mediante patrones contextuales, clasificación de oraciones, generación de diálogos y persistencia con SQLite.

## Características

- **Análisis morfológico** – género, número, tiempo, persona, grado basado en sufijos y listas de palabras.
- **Etiquetado HMM** – Viterbi y forward‑backward con probabilidades posteriores.
- **Memoria contextual** – aprende de correcciones y refuerza clasificaciones correctas en contextos específicos.
- **Correlación de patrones** – registra trigramas (palabras anteriores → palabra actual → palabra siguiente) para predecir continuaciones.
- **Gestión de diálogos** – almacena premisas, hipótesis y patrones creativos; puede generar respuestas.
- **Persistencia SQLite** – todas las palabras, oraciones, patrones, diálogos y memoria contextual se almacenan en dos bases de datos separadas (BD semántica y BD de correlaciones).
- **Aprendizaje interactivo** – el sistema mejora a partir de la retroalimentación del usuario durante la ejecución.

## Dependencias

- Compilador con **C++17** (GCC 7+, Clang 6+, MSVC 2019+)
- **SQLite3** (bibliotecas de desarrollo)
  - Ubuntu/Debian: `sudo apt install libsqlite3-dev`
  - macOS (Homebrew): `brew install sqlite3`
  - Windows: usar [vcpkg](https://vcpkg.io) `vcpkg install sqlite3` o descargar las DLL manualmente

## Compilación

1. **Clonar o copiar el código fuente** en un directorio (ej. `motor-nlp/`).
2. Crear un directorio de build y ejecutar CMake:

```bash
mkdir build && cd build
cmake ..
cmake --build .

    El ejecutable motor_nlp se generará en build/ (o build/bin/ si se descomenta la línea de directorio de salida).

Uso
Inicialización

El motor espera dos archivos de base de datos SQLite:

    nlp_semantic.db – almacena palabras, oraciones, patrones, diálogos, retroalimentación y memoria contextual.

    nlp_correlations.db – almacena las tablas de correlación de patrones para la predicción de siguiente palabra.

Si los archivos no existen, se crean automáticamente.
Ejecutar la demo interactiva

Después de compilar, ejecute el programa desde la raíz del proyecto (para que las bases de datos se creen en el directorio de trabajo):
bash

./build/motor_nlp

La demo:

    Aprenderá desde un archivo opcional aprende.txt (una oración por línea).

    Entrará en un bucle interactivo donde usted proporciona una frase inicial (≥2 palabras).

    En cada iteración, el motor predice la siguiente palabra, usted confirma o corrige, y el sistema aprende la secuencia correcta.

    Después de 5 iteraciones, se muestra la frase final generada.

Resumen de la API (clase NLPEngine)
Método	Descripción
initialize(semanticDb, correlatorDb)	Abre o crea las dos bases de datos SQLite.
learnText(texto)	Introduce una oración al sistema; actualiza correlaciones de patrones y estadísticas de palabras.
classifySentence(oracion)	Devuelve información morfológica y de etiqueta POS para cada palabra.
predictNextWords(actual, palabrasAnteriores)	Predice posibles siguientes palabras dada una palabra y su contexto (hasta dos anteriores).
generateHypothesis(premisa)	Produce una respuesta creativa basada en la premisa.
correctWord(palabra, tipoCorrecto)	Corrige manualmente la etiqueta POS de una palabra y actualiza todos los modelos internos.

Para más detalles de la API, consulte nlp_api.h.
Estructura del Proyecto
text

├── CMakeLists.txt
├── README.md
├── main.cpp                      # Demo interactiva
├── nlp_api.h / nlp_api.cpp       # Fachada de la API pública
├── core/                         # Word, ContextualCorrelator, helpers de aprendizaje
├── dialogue/                     # Pattern, PatternCorrelator, DialogueHistory
├── db/                           # Repositorios SQLite y gestor de BD
├── nlp/                          # Classifier, HMM, morfología, estadísticas de corpus, memoria contextual
├── tokenizer/                    # Tokenizador UTF‑8 (números, fechas, palabras)
├── utils/                        # Conversiones de cadena, serialización de patrones
└── types.hpp                     # Enumeraciones globales (TipoPalabra, etc.)

Extensión del Motor

    Añadir nuevas palabras – El sistema las aprende automáticamente al encontrarlas. Las clasificaciones se refinan mediante correctWord().

    Entrenamiento con grandes corpus – Llame a learnText() repetidamente con oraciones; las probabilidades de emisión y transición del HMM se actualizan incrementalmente.

    Memoria contextual – La clase ContextMemory almacena asociaciones palabra‑contexto. Use cleanup() para eliminar entradas antiguas o de baja confianza.

Licencia

Este proyecto se proporciona "tal cual". Para licencias personalizadas, contacte al autor.

Autor – Soubhi Khayat Najjar kayayox@gmail.com
Última actualización – Abril 2026
