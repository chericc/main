set(DEMO demo_wav_muxer)
set(LIB_NAME wav_muxer)

aux_source_directory(. src_wav_muxer)
aux_source_directory(demo src_wav_muxer_demo)

include_directories(
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${G_XTOOLS_XIO_HEADER_PATH}
    .
)

add_library(${LIB_NAME} STATIC
    ${src_wav_muxer})

target_link_libraries(${LIB_NAME}
    ${G_XTOOLS})

add_executable(${DEMO} ${src_wav_muxer_demo})
target_link_libraries(
    ${DEMO} 
    ${LIB_NAME} 
    ${G_XTOOLS}
)