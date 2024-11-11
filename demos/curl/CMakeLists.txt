set(DEMO_NAME demo_curl)

aux_source_directory(. src)

include_directories(${G_XTOOLS_XLOG_HEADER_PATH})
link_libraries(${G_XTOOLS} ${LIB_CURL})

add_executable(${DEMO_NAME}
    ${src})
