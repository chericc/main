set(DEMO_NAME demo_curl)

if (WITH_CURL)

aux_source_directory(. src)

include_directories(${G_XTOOLS_XLOG_HEADER_PATH} ${G_CURL_INC_PATH})
link_directories(${G_CURL_LIB_PATH})
link_libraries(${G_XTOOLS} ${G_CURL_LIBNAME})

add_executable(${DEMO_NAME}
    ${src})

endif()