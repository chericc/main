set(DEMO_NAME demo_h264_parser)

aux_source_directory(. src)

link_libraries(${G_XTOOLS})

add_executable(${DEMO_NAME}
    ${src})
