set(DEMO_NAME demo_cjson)

if (WITH_CJSON)

aux_source_directory(. src_demo_cjson)

include_directories(${G_XTOOLS_XLOG_HEADER_PATH} ${G_CJSON_INC_PATH})
link_directories(${G_CJSON_LIB_PATH})
link_libraries(${G_XTOOLS} ${G_CJSON_LIBS})

add_executable(${DEMO_NAME}
    ${src_demo_cjson})

endif()