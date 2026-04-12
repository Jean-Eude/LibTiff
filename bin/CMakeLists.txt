cmake_minimum_required(VERSION 3.16)
project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Headers .hpp ou .h
include_directories(../include ../include/libtiff ../include/tiffdp ../include/matplot)

# Librairies en .so
link_directories(../lib)

# Liste des fichiers sources .cpp ou .c
file(GLOB SOURCES  ../*.cpp ../src/*.cpp)

# Executable
add_executable(main ${SOURCES})

# Drapeaux de compilation 
target_compile_options(main PRIVATE -Wall -O3)

# Link des librairies 
target_link_libraries(main PRIVATE -ltiff matplot)