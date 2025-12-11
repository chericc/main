aux_source_directory(. src)

link_libraries(
    ${G_XTOOLS}
    sqlpp11::sqlpp11
    ${LIB_SQLITE3}
    pthread
    dl
    m
)
include_directories(
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${G_XTOOLS_XIO_HEADER_PATH}
    ${HEADER_SQLITE3}
)

set(BIN demo_sqlpp11)
add_executable(
    ${BIN}
    ${src}
)
target_link_libraries(
    ${BIN}
    pthread
)
install(
    TARGETS ${BIN}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)