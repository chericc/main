
aux_source_directory(. src_md5dir)

add_executable(
    ${G_MD5DIR}
    ${src_md5dir}
)

if(${gnu})
set(COMMON_LIB  pthread)
elseif(${msvc})
set(COMMON_LIB )
elseif(${clang})
set(COMMON_LIB )
endif()

target_link_libraries(
    ${G_MD5DIR}
    ${COMMON_LIB}
)