set(DEMO_RTSP_CLIENT demo_rtsp_client)
aux_source_directory(. src)

set(LIVE555_LIBS 
    ${LIB_LIVE555_BasicUsageEnvironment}
    ${LIB_LIVE555_groupsock}
    ${LIB_LIVE555_liveMedia}
    ${LIB_LIVE555_UsageEnvironment})

include_directories(${G_XTOOLS_XLOG_HEADER_PATH} ${HEADER_LIB_LIVE555})
link_libraries(${G_XTOOLS} ${LIVE555_LIBS})

add_executable(${DEMO_RTSP_CLIENT}
    ${src})