cmake_minimum_required(VERSION 3.1)
project(re216)

add_compile_options(-Wall -g -O0)

# Ajoute les fichiers sources du projet.
file(GLOB SOURCES
    src/common.h
    src/structs.h
    src/*.c
    src/*.h
    src/*/*.c
    src/*/*.h
)

file(GLOB SERVER_SOURCES
    src/server.c
    ${COMMON_SOURCES}
)

file(GLOB CLIENT_SOURCES
    src/client.c
    ${COMMON_SOURCES}
)

# Définit le répertoire de sortie des fichiers exécutables.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

# Crée l'exécutable du serveur à partir des fichiers sources.
add_executable(server ${SERVER_SOURCES})

# Crée l'exécutable du client à partir des fichiers sources.
add_executable(client ${CLIENT_SOURCES})

