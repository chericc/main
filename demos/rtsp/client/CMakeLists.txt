set(DEMO_RTSP_CLIENT demo_rtsp_client)
aux_source_directory(. src)

include_directories(${G_XTOOLS_XLOG_HEADER_PATH})
link_libraries(${G_XTOOLS})

add_executable(${DEMO_RTSP_CLIENT}
    ${src}
    ${src_live555})