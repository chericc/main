aux_source_directory(. src)

link_libraries(
    ${G_XTOOLS}
)
include_directories(
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${G_XTOOLS_XUTILITY_HEADER_PATH}
)

set(BIN demo_fmtlib)
add_executable(
    ${BIN}
    ${src}
)
install(
    TARGETS ${BIN}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)