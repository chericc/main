aux_source_directory(. src_lib)

link_libraries(
    ${G_XTOOLS}
)

add_library(${G_XDEMUXER} STATIC ${src_lib})
install(
    TARGETS ${G_XDEMUXER}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

aux_source_directory(./demo src_demo)

set(COMMON_LIBS ${LIB_FFMPEG_AVFORMAT} ${LIB_FFMPEG_AVUTIL} ${LIB_FFMPEG_AVCODEC})

include_directories(
    .
    ${HEADER_FFMPEG}
    ${G_XTOOLS_XLOG_HEADER_PATH}
)

set(DEMO demo_${G_XDEMUXER})
add_executable(${DEMO} ${src_demo})
target_link_libraries(
    ${DEMO} 
    ${G_XDEMUXER}
    ${COMMON_LIBS}
)
install(
    TARGETS ${DEMO}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)