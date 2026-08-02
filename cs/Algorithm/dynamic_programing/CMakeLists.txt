# Sub-build for the dynamic programming examples.

# Collect sources recursively so each example can live in its own
# sub-folder (e.g. longest_common_subsequence/).
file(GLOB_RECURSE src_dp CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

# Object library so that every object file is always linked into the
# final executable.  This matters here: the demos register themselves
# through static initializers (StaticRegistrant), and a plain static
# library would let the linker drop unreferenced objects (e.g. longest_common_subsequence).
add_library(alg_dp OBJECT ${src_dp})

target_include_directories(alg_dp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(alg_dp PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/main
)
