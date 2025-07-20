set(L_XPLAY_TEST test_xplay)

aux_source_directory(. src_xplay)
aux_source_directory(test src_xplay_test)

message("HEADER" ${HEADER_FFMPEG})

include_directories(
    ${HEADER_FFMPEG}
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${G_XPLAY_HEADER_PATH}
    ${G_XTOOLS_XTHREAD_HEADER_PATH}
    ${G_XTOOLS_XUTILITY_HEADER_PATH}
)

# ignore now
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

add_library(
    ${G_XPLAY} SHARED ${src_xplay}
)

set(LIB_FFMPEG 
    ${LIB_FFMPEG_AVFORMAT}
    ${LIB_FFMPEG_AVCODEC}
    ${LIB_FFMPEG_AVUTIL}
    ${LIB_FFMPEG_SWRESAMPLE}
    ${LIB_FFMPEG_SWSCALE}
)

target_link_libraries(
    ${G_XPLAY}
    ${G_XTOOLS}
    ${LIB_FFMPEG}
)
