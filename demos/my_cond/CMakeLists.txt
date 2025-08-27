aux_source_directory(. src)

set(DEMO_NAME demo_my_cond)
add_executable(${DEMO_NAME} ${src})

add_definitions(-DUSE_STD_COND)

target_include_directories(
    ${DEMO_NAME} 
    PRIVATE ${G_XTOOLS_XLOG_HEADER_PATH}
)
target_link_libraries(
    ${DEMO_NAME}
    ${G_XTOOLS}
)