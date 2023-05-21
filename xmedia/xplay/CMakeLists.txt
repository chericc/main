set(L_XPLAY_TEST test_xplay)

aux_source_directory(. src_xplay)
aux_source_directory(test src_xplay_test)

include_directories(
    ${G_FFMPEG_INC_PATH}
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${G_GOOGLETEST_INC_PATH}
    ${G_XPLAY_HEADER_PATH}
)

link_directories(
    ${G_FFMPEG_LIB_PATH}
)

add_library(
    ${G_XPLAY} SHARED ${src_xplay}
)
target_link_libraries(
    ${G_XPLAY}
    ${G_XTOOLS}
    avformat swscale avformat avcodec avutil swresample
)

if(${gnu})
set(COMMON_LIB 
    ${G_XTOOLS}
    avformat swscale avformat avcodec avutil swresample 
    gtest
    pthread)
elseif(${msvc})
set(COMMON_LIB 
    ${G_XTOOLS}
    avformat swscale avformat avcodec avutil swresample
    gtest)
elseif(${clang})
set(COMMON_LIB 
    ${G_XTOOLS}
    avformat swscale avformat avcodec avutil swresample
    gtest)
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