
aux_source_directory(. src_wav_demuxer)

include_directories(
    ${G_XTOOLS_XLOG_HEADER_PATH}
)

add_library(${G_XWAV_DEMUXER}
    SHARED
    ${src_wav_demuxer})

# tests

