set(DEMO_NAME demo_rtsp_client)
set(RTSP_LIB rtsp_client)

include_directories(
    ${G_XDEMUXER_HEADER_PATH}
    ${HEADER_FFMPEG}
    ${HEADER_LIVE555}
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${HEADER_LIVE555}/BasicUsageEnvironment
    ${HEADER_LIVE555}/groupsock
    ${HEADER_LIVE555}/liveMedia
    ${HEADER_LIVE555}/UsageEnvironment
)

link_directories(${LIB_LIVE555_PATH})

add_library(${RTSP_LIB} STATIC
    rtsp_client.cpp)

target_link_libraries(${RTSP_LIB} PUBLIC
    ${LIB_LIVE555}
    ${G_XTOOLS}
    spdlog::spdlog
    pthread
)

add_executable(${DEMO_NAME}
    main.cpp)

target_link_libraries(${DEMO_NAME}
    ${RTSP_LIB}
    ${G_XTOOLS}
    ${LIB_FFMPEG_AVFORMAT}
    ${LIB_FFMPEG_AVCODEC}
    ${LIB_FFMPEG_AVUTIL}
    spdlog::spdlog
    pthread)

install(
    TARGETS ${RTSP_LIB}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(
    TARGETS ${DEMO_NAME}
    RUNTIME DESTINATION bin
)
