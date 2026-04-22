cmake_minimum_required(VERSION 3.10)
project(MotorNLP VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SQLite3 REQUIRED)

file(GLOB_RECURSE SOURCES "*.cpp")

list(FILTER SOURCES EXCLUDE REGEX ".*/build/.*")

if(NOT SOURCES)
    message(WARNING "No se encontraron archivos .cpp. Verifica la estructura del proyecto.")
endif()

add_executable(motor_nlp ${SOURCES})

target_include_directories(motor_nlp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(motor_nlp PRIVATE SQLite::SQLite3)

add_custom_command(TARGET motor_nlp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/aprende.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/word_freq.txt
        ${CMAKE_CURRENT_BINARY_DIR}/
    COMMENT "Copiando archivos a build/"
)
