set(L_TESTENC_UTF8_TEST test_textenc_utf8)

aux_source_directory(utf8_enc src_utf8)
aux_source_directory(utf8_enc/test src_utf8_test)

include_directories(
    ${G_TEXTENC_UTF8_HEADER_PATH}
    ${G_XTOOLS_XLOG_HEADER_PATH}
)

add_library(
    ${G_TEXTENC}
    SHARED
    ${src_utf8}
)

if(${gnu})
set(COMMON_LIB ${G_GOOGLETEST} ${G_XTOOLS} ${G_TEXTENC} pthread)
elseif(${msvc})
set(COMMON_LIB ${G_GOOGLETEST} ${G_XTOOLS} ${G_TEXTENC} )
elseif(${clang})
set(COMMON_LIB ${G_GOOGLETEST} ${G_XTOOLS} ${G_TEXTENC} )
endif()

add_executable(${L_TESTENC_UTF8_TEST}
    ${src_utf8_test}
    ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${L_TESTENC_UTF8_TEST}
    ${COMMON_LIB})
add_test(NAME ${L_TESTENC_UTF8_TEST}
        COMMAND ${L_TESTENC_UTF8_TEST})