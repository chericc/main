set(ALG demo_alg)

aux_source_directory(main src_demo_alg_main)

include_directories(${G_XTOOLS_XLOG_HEADER_PATH})

link_directories(${G_XTOOLS})
link_libraries(${G_XTOOLS})

add_executable(${ALG} ${src_demo_alg_main})