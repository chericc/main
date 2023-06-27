set(FFMPEG_FORMAT_DEMO demo_ffmpeg_format)

aux_source_directory(format src_format)

add_executable(${FFMPEG_FORMAT_DEMO}
    ${src_format})

target_include_directories(${FFMPEG_FORMAT_DEMO}
    PRIVATE ${G_FFMPEG_INC_PATH}
    PRIVATE ${G_XTOOLS_XLOG_HEADER_PATH})

target_link_directories(${FFMPEG_FORMAT_DEMO}
    PRIVATE ${G_FFMPEG_LIB_PATH}
    )

target_link_libraries(${FFMPEG_FORMAT_DEMO}
    avformat avutil xtools)

