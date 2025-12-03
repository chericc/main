aux_source_directory(. src)

link_libraries(
    ${G_XTOOLS}
    sqlpp11::sqlpp11
)
include_directories(${G_XTOOLS_XLOG_HEADER_PATH})

set(BIN demo_sqlpp11)
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