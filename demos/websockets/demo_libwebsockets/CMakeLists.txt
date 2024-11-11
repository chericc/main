set(DEMO_LWS demo_lws)
aux_source_directory(. src)

include_directories(${G_XTOOLS_XLOG_HEADER_PATH})
link_libraries(${G_XTOOLS} ${LIB_WEBSOCKETS})

add_executable(${DEMO_LWS}
    ${src})