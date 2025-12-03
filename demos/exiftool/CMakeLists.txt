aux_source_directory(. src)

include_directories(
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${G_XTOOLS_XCROSSPLATFORM_HEADER_PATH}
)
link_libraries(
    ${G_XTOOLS}
)

set(BIN demo_exiftool)
add_executable(${BIN} ${src})