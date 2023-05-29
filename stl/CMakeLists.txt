
set(STL_DEMO demo_stl)

aux_source_directory(src src_stl_demo)

add_executable(
    ${STL_DEMO}
    ${src_stl_demo}
)
