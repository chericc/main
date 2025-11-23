set(BIN demo_rtsp_server_live555)
aux_source_directory(. src)

set(COMMON_LIBS ${LIB_FFMPEG_AVFORMAT} ${LIB_FFMPEG_AVUTIL} ${LIB_FFMPEG_AVCODEC})

include_directories(
    ${HEADER_FFMPEG}
    ${G_XDEMUXER_HEADER_PATH}
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${HEADER_BOOST}
)
link_libraries(
    ${G_XDEMUXER}
    ${G_XTOOLS}
    ${COMMON_LIBS}
)

add_executable(
    ${BIN}
    ${src}
)
install(
    TARGETS ${BIN}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)