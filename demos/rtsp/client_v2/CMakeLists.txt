set(DEMO_RTSP_WRAP demo_rtsp_client_v2)
aux_source_directory(. src)

include_directories(${G_XTOOLS_XLOG_HEADER_PATH})
link_libraries(${G_XTOOLS})

add_executable(${DEMO_RTSP_WRAP}
    ${src}
    $<TARGET_OBJECTS:lib555_objects>)

