set(DEMO_LWS demo_lws)
aux_source_directory(. src)

include_directories(${G_XTOOLS_XLOG_HEADER_PATH} ${G_LWS_INC_PATH})
link_libraries(${G_XTOOLS} ${G_LWSNAME})
link_directories(${G_LWS_LIB_PATH})

add_executable(${DEMO_LWS}
    ${src})