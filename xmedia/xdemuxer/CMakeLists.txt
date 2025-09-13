aux_source_directory(. src_lib)

add_library(${G_XDEMUXER} STATIC ${src_lib})

aux_source_directory(./demo src_demo)

set(COMMON_LIBS ${LIB_FFMPEG_AVFORMAT} ${LIB_FFMPEG_AVUTIL})

include_directories(
    .
    ${HEADER_FFMPEG}
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${COMMON_LIBS}
)

set(DEMO demo_${G_XDEMUXER})
add_executable(${DEMO} ${src_demo})
target_link_libraries(
    ${DEMO} 
    ${G_XDEMUXER}
    ${G_XTOOLS}
    ${COMMON_LIBS}
)