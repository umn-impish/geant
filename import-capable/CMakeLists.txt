cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 20)
project(cadmesh-wrapper)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Geant4 REQUIRED ui_all vis_all)
message(${Geant4_USE_FILE})
include(${Geant4_USE_FILE})

add_subdirectory(src)

find_package(nlohmann_json REQUIRED)
target_link_libraries(
    cadmesh-wrapper
    ${Geant4_LIBRARIES}
    tet
    nlohmann_json::nlohmann_json
)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data-out")
file(CREATE_LINK "${CMAKE_SOURCE_DIR}/macros" "${CMAKE_CURRENT_BINARY_DIR}/macros" SYMBOLIC)

add_compile_options(-Wall -Wextra -Wpedantic -pedantic -Werror)
add_compile_definitions(CMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}")
