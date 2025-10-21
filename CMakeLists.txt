cmake_minimum_required(VERSION 3.27.0)

project(
    stego
    VERSION 0.1
    LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    src/main.c
    src/png.h
    src/png.c
    src/utils.h
    src/utils.c
)

add_executable(
    ${PROJECT_NAME} 
    ${SOURCES}
)

find_package(ZLIB REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC ZLIB::ZLIB)
