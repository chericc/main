set(DEMO_RTSP_CLIENT demo_rtsp_server)
aux_source_directory(. src)

include_directories(${G_XTOOLS_XLOG_HEADER_PATH} ${HEADER_LIB_LIVE555})
link_libraries(${G_XTOOLS} ${LIB_LIVE555})

add_executable(${DEMO_RTSP_CLIENT}
    ${src})