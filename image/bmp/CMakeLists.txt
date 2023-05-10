
set(L_BMP_TEST test_bmp)

aux_source_directory(. src_bmp)
aux_source_directory(test src_bmp_test)

include_directories(
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${G_XTOOLS_XIO_HEADER_PATH}
    ${G_BMP_HEADER_PATH}
)

link_directories(${GTEST_PATH}/lib)

add_library(${G_BMP} SHARED ${src_bmp})
target_link_libraries(${G_BMP}
    ${G_XTOOLS})

if(${gnu})
set(COMMON_LIB gtest ${G_BMP} ${G_XTOOLS} pthread)
elseif(${msvc})
set(COMMON_LIB gtest ${G_BMP} ${G_XTOOLS})
endif()

add_executable(${L_BMP_TEST}
    ${src_bmp_test}
    ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${L_BMP_TEST}
    ${COMMON_LIB})