add_executable(
    basic-optics
        main.cc
        actions.cc
        analysis.cc
        construction.cc
        detectors.cc
        hits.cc
)

target_include_directories(
    basic-optics
    PUBLIC
    .
)

add_subdirectory(materials)
add_subdirectory(config)
