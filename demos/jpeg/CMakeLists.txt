aux_source_directory(. src)

include_directories(
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${HEADER_JPEG}
)
link_libraries(
    ${G_XTOOLS}
    ${LIB_JPEG}
)

set(BIN demo_jpeg)
add_executable(
    ${BIN}
    ${src}
)