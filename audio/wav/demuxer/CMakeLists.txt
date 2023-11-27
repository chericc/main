# variable

set(WAV_DEMUXER_TEST test_wav_demuxer)

# targets

aux_source_directory(. src_wav_demuxer)
aux_source_directory(test src_wav_demuxer_test)

include_directories(
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${G_XTOOLS_XIO_HEADER_PATH}
    ${G_XWAV_DEMUXER_HEADER_PATH}
)

add_library(${G_XWAV_DEMUXER}
    SHARED
    ${src_wav_demuxer})
target_link_libraries(${G_XWAV_DEMUXER}
    ${G_XTOOLS})

# tests

if(${gnu})
set(COMMON_LIB gtest ${G_TEST_COMM} ${G_XWAV_DEMUXER} ${G_XTOOLS} pthread)
elseif(${msvc})
set(COMMON_LIB gtest ${G_TEST_COMM} ${G_XWAV_DEMUXER} ${G_XTOOLS})
elseif(${clang})
set(COMMON_LIB gtest ${G_TEST_COMM} ${G_XWAV_DEMUXER} ${G_XTOOLS})
endif()

add_definitions(-DRES_AUDIO_PATH="${G_RESOURCE_PATH}/audio")

add_executable(${WAV_DEMUXER_TEST}
    ${src_wav_demuxer_test}
    ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${WAV_DEMUXER_TEST}
    ${COMMON_LIB})
target_include_directories(${WAV_DEMUXER_TEST}
    PRIVATE ${G_TSET_COMM_HEADER_PATH})
add_test(NAME ${WAV_DEMUXER_TEST}
    COMMAND ${WAV_DEMUXER_TEST})
