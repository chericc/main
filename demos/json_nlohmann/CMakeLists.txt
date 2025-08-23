
aux_source_directory(. src)

include_directories(${G_XTOOLS_XLOG_HEADER_PATH})
link_libraries(${G_XTOOLS})

add_compile_options(-Wno-unused-function)

add_executable(demo_json_cpp
    ${src})