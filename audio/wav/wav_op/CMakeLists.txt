set(DEMO demo_wav_muxer)
set(LIB_NAME wav_muxer)
set(TEST_WAV_DEMUXER test_wav_demuxer)

aux_source_directory(. src_wav_muxer)
aux_source_directory(demo_wav_muxer src_wav_muxer_demo)

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

# tests
if(LIB_GTEST)

if(${gnu})
set(COMMON_LIB ${LIB_GTEST} ${G_TEST_COMM} ${LIB_NAME} ${G_XTOOLS} pthread)
elseif(${msvc})
set(COMMON_LIB ${LIB_GTEST} ${G_TEST_COMM} ${LIB_NAME} ${G_XTOOLS})
elseif(${clang})
set(COMMON_LIB ${LIB_GTEST} ${G_TEST_COMM} ${LIB_NAME} ${G_XTOOLS})
endif()

add_definitions(-DRES_AUDIO_PATH="${G_RESOURCE_PATH}/audio")

aux_source_directory(test_wav_muxer src_wav_demuxer_test)

add_executable(${TEST_WAV_DEMUXER}
    ${src_wav_demuxer_test}
    ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${TEST_WAV_DEMUXER}
    ${COMMON_LIB})
target_include_directories(${TEST_WAV_DEMUXER}
    PRIVATE ${G_TSET_COMM_HEADER_PATH}
    PRIVATE ${HEADER_GTEST})
add_test(NAME ${TEST_WAV_DEMUXER}
    COMMAND ${TEST_WAV_DEMUXER})

endif()