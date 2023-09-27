set(FFMPEG_FORMAT_DEMO demo_ffmpeg_format)
set(FFMPEG_MUXER_DEMO demo_ffmpeg_muxer)

aux_source_directory(format src_format)
aux_source_directory(muxer src_muxer)

add_executable(${FFMPEG_FORMAT_DEMO}
    ${src_format})


add_executable(${FFMPEG_MUXER_DEMO}
    ${src_muxer})
target_include_directories(${FFMPEG_FORMAT_DEMO}
    PRIVATE ${G_FFMPEG_INC_PATH}
    PRIVATE ${G_XTOOLS_XLOG_HEADER_PATH})
target_link_directories(${FFMPEG_FORMAT_DEMO}
    PRIVATE ${G_FFMPEG_LIB_PATH}
    )
target_link_libraries(${FFMPEG_FORMAT_DEMO}
    avformat avutil xtools)

target_include_directories(${FFMPEG_MUXER_DEMO}
    PRIVATE ${G_FFMPEG_INC_PATH}
    PRIVATE ${G_XTOOLS_XLOG_HEADER_PATH})
target_link_directories(${FFMPEG_MUXER_DEMO}
    PRIVATE ${G_FFMPEG_LIB_PATH}
    )
target_link_libraries(${FFMPEG_MUXER_DEMO}
    avformat avutil xtools)