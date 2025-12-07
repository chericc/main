aux_source_directory(. src)

set(DEMO demo_sqlite)
add_executable(${DEMO} ${src})
target_include_directories(${DEMO} 
    PRIVATE ${HEADER_SQLITE3})
target_link_libraries(
    ${DEMO}
    ${LIB_SQLITE3}
    m
)