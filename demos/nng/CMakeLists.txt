aux_source_directory(. src)

include_directories(
    .
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${HEADER_NNG}
)

link_libraries(
    ${G_XTOOLS}
    ${LIB_NNG}
    pthread
)
add_executable(
    demo_nng
    ${src}
)