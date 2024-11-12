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

if(LIB_GTEST)

set(COMMON_LIB 
    ${G_XTOOLS}
    avformat swscale avformat avcodec avutil swresample 
    ${G_GOOGLETEST})
if(${gnu})
set(COMMON_LIB ${COMMON_LIB} pthread)
endif()

add_definitions(-DRES_VIDEO_PATH="${G_RESOURCE_PATH}/video")

add_executable(
    ${L_XPLAY_TEST}
    ${src_xplay_test}
    ${G_GOOGLETEST_MAIN_SRCFILE}
)

target_link_libraries(
    ${L_XPLAY_TEST} 
    ${COMMON_LIB}
)

add_test(NAME ${L_XPLAY_TEST}
        COMMAND ${L_XPLAY_TEST})

endif()