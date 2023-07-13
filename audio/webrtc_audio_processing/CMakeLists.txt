project(main)
cmake_minimum_required(VERSION 3.0)

set(LIBNAME_WEBTRC_AUDIO webrtc_audio)

# paths
set(MODULE_AUDIO_PROCESSING_PATH ${CMAKE_SOURCE_DIR}/modules/audio_processing)
set(MODULE_AUDIO_PROCESSING_AGC_PATH ${MODULE_AUDIO_PROCESSING_PATH}/agc)
set(MODULE_AUDIO_PROCESSING_VAD_PATH ${MODULE_AUDIO_PROCESSING_PATH}/vad)
set(COMMON_AUDIO_PATH ${CMAKE_SOURCE_DIR}/common_audio)
set(COMMON_AUDIO_VAD_PATH ${COMMON_AUDIO_PATH}/vad)
set(COMMON_AUDIO_SIGNAL_PROCESSING ${COMMON_AUDIO_PATH}/signal_processing)
set(COMMON_AUDIO_THIRD_PARTY_OOURA_FFT_SIZE_256_PATH ${COMMON_AUDIO_PATH}/third_party/ooura/fft_size_256)
set(COMMON_AUDIO_RESAMPLER_PATH ${COMMON_AUDIO_PATH}/resampler)
set(MODULE_AUDIO_CODING_CODECS_ISAC_PATH ${CMAKE_SOURCE_DIR}/modules/audio_coding/codecs/isac)

# deps/lib-inc/abseil
set(ABSEIL_INCLUDE_PATH /home/test/opensrc/abseil/build/output/include)
set(ABSEIL_LIB_PATH /home/test/opensrc/abseil/build/output/lib)

# source
set(SRC_AGC
    ${MODULE_AUDIO_PROCESSING_AGC_PATH}/agc_manager_direct.cc
    ${MODULE_AUDIO_PROCESSING_AGC_PATH}/agc.cc
    ${MODULE_AUDIO_PROCESSING_AGC_PATH}/loudness_histogram.cc
    ${MODULE_AUDIO_PROCESSING_AGC_PATH}/utility.cc
)

set(SRC_VAD
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/voice_activity_detector.cc
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/pitch_based_vad.cc
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/vad_audio_proc.cc
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/standalone_vad.cc
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/vad_circular_buffer.cc
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/gmm.cc
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/pole_zero_filter.cc
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/pitch_internal.cc
)

set(SRC_COMMON_AUDIO_VAD
    ${COMMON_AUDIO_VAD_PATH}/webrtc_vad.c
    ${COMMON_AUDIO_VAD_PATH}/vad_core.c
    ${COMMON_AUDIO_VAD_PATH}/vad_gmm.c
    ${COMMON_AUDIO_VAD_PATH}/vad_sp.c
    ${COMMON_AUDIO_VAD_PATH}/vad_filterbank.c
)

set(SRC_SIGNAL_PROCESSING
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/resample_48khz.c
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/resample.c
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/resample_by_2.c
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/division_operations.c
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/energy.c
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/resample_by_2_internal.c
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/resample_fractional.c
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/get_scaling_square.c)

set(SRC_ISAC
    ${MODULE_AUDIO_CODING_CODECS_ISAC_PATH}/isac_vad.c
    ${MODULE_AUDIO_CODING_CODECS_ISAC_PATH}/filter_functions.c
    ${MODULE_AUDIO_CODING_CODECS_ISAC_PATH}/pitch_estimator.c
    ${MODULE_AUDIO_CODING_CODECS_ISAC_PATH}/pitch_filter.c)

set(SRC_THIRD_OOURA
    ${COMMON_AUDIO_THIRD_PARTY_OOURA_FFT_SIZE_256_PATH}/fft4g.cc)

set(SRC_RESAMPLE
    ${COMMON_AUDIO_RESAMPLER_PATH}/resampler.cc)

add_library(${LIBNAME_WEBTRC_AUDIO} SHARED
    ${SRC_AGC}
    ${SRC_VAD}
    ${SRC_COMMON_AUDIO_VAD})

target_include_directories(
    ${LIBNAME_WEBTRC_AUDIO}
    PRIVATE ${ABSEIL_INCLUDE_PATH}
    PRIVATE ${CMAKE_SOURCE_DIR}
)

target_compile_definitions(${LIBNAME_WEBTRC_AUDIO}
    PRIVATE WEBRTC_APM_DEBUG_DUMP=0
    PRIVATE WEBRTC_POSIX)

# add_executable()