
aux_source_directory(. src_md5dir)

include_directories(
    ${G_XTOOLS_XLOG_HEADER_PATH}
)

add_executable(
    ${G_MD5DIR}
    ${src_md5dir}
)

if(${gnu})
set(COMMON_LIB ${G_XTOOLS} pthread)
elseif(${msvc})
set(COMMON_LIB ${G_XTOOLS})
elseif(${clang})
set(COMMON_LIB ${G_XTOOLS})
endif()

target_link_libraries(
    ${G_MD5DIR}
    ${COMMON_LIB}
)