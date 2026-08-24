add_library(
    tetgen STATIC
    tetgen/tetgen.cxx
    tetgen/predicates.cxx
)

target_include_directories(
    tetgen
    PUBLIC
    tetgen
)

# Required to build tetgen as a library
target_compile_definitions(
    tetgen PRIVATE TETLIBRARY=1
)
