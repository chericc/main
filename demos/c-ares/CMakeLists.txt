aux_source_directory(. src)

set(DEMO demo_cares)
add_executable(${DEMO} ${src})
target_link_libraries(
    ${DEMO}
    ${G_XTOOLS}
    ${LIB_C_ARES}
)
target_include_directories(
    ${DEMO}
    PRIVATE ${G_XTOOLS_XLOG_HEADER_PATH}
    PRIVATE ${HEADER_C_ARES}
)