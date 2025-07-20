aux_source_directory(. src)
aux_source_directory(./demo src_demo)

set(DEMO_NAME demo_xtea)

add_executable(${DEMO_NAME} 
    ${src} 
    ${src_demo}
)

target_link_libraries(
    ${DEMO_NAME}
    ${G_XTOOLS}
)

target_include_directories(
    ${DEMO_NAME}
    PRIVATE .
    PRIVATE ${G_XTOOLS_XLOG_HEADER_PATH}
)