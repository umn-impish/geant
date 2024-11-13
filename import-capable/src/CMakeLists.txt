add_executable(
    cadmesh-wrapper
        main.cc
        actions.cc
        analysis.cc
        construction.cc
        detectors.cc
        hits.cc
)

target_include_directories(
    cadmesh-wrapper
    PUBLIC
    .
)

add_subdirectory(materials)
add_subdirectory(config)
