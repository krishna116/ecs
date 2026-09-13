cmake_minimum_required(VERSION 4.0)

project(ecs VERSION 1.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

configure_file(
${PROJECT_SOURCE_DIR}/cmake/config.h.txt
${PROJECT_SOURCE_DIR}/src/config.h
)

add_library(ecsCompilerLib
src/CppParser.cpp
src/EcsCompiler.cpp
)

target_include_directories(ecsCompilerLib PUBLIC
src
)

add_subdirectory(tests)