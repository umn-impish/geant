cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 20)
project(cadmesh-wrapper)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Geant4 REQUIRED ui_all vis_all)
message(${Geant4_USE_FILE})
include(${Geant4_USE_FILE})

add_subdirectory(extern)
add_subdirectory(src)

include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)

# MessagePack: use for very large meta files
FetchContent_Declare(
    msgpack
    GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
    GIT_TAG        cpp-9.0.0
)
FetchContent_MakeAvailable(msgpack)

FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG        v1.4
)
FetchContent_MakeAvailable(zlib)


target_link_libraries(
    cadmesh-wrapper
    ${Geant4_LIBRARIES}
    tetgen
    nlohmann_json::nlohmann_json
    msgpack-cxx
    zlib
)

target_compile_options(cadmesh-wrapper PRIVATE -Wall -Wextra -Wpedantic -Werror)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data-out")
file(CREATE_LINK "${CMAKE_SOURCE_DIR}/macros" "${CMAKE_CURRENT_BINARY_DIR}/macros" SYMBOLIC)
file(CREATE_LINK "${CMAKE_SOURCE_DIR}/simulation.config.file" "${CMAKE_CURRENT_BINARY_DIR}/simulation.config.file" SYMBOLIC)

add_compile_options(-Wall -Wextra -Wpedantic -pedantic -Werror)
add_compile_definitions(CMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}")
