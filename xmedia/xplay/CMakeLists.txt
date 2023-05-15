
set(L_XPLAY xplay)

aux_source_directory(. src_xplay)

link_directories(
    ${G_FFMPEG_LIB_PATH}
)

add_definitions(-DRES_VIDEO_PATH="${G_RESOURCE_PATH}/video")

add_executable(
    ${L_XPLAY} 
    ${src_xplay}
)

target_include_directories(
    ${L_XPLAY} 
    PRIVATE ${G_FFMPEG_INC_PATH}
    PRIVATE ${G_XTOOLS_XLOG_HEADER_PATH}
)

if(${gnu})
set(COMMON_LIB 
    ${G_XTOOLS}
    avformat swscale avformat avcodec avutil swresample 
    pthread)
elseif(${msvc})
set(COMMON_LIB 
    ${G_XTOOLS}
    avformat swscale avformat avcodec avutil swresample )
endif()

target_link_libraries(
    ${L_XPLAY} 
    ${COMMON_LIB}
)
