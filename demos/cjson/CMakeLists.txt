set(DEMO_NAME demo_cjson)

aux_source_directory(. src_demo_cjson)

include_directories(${G_XTOOLS_XLOG_HEADER_PATH} ${HEADER_CJSON})
link_libraries(${G_XTOOLS} ${LIB_CJSON})

add_executable(${DEMO_NAME}
    ${src_demo_cjson})
