
set(L_IMAGE_BMP_TEST test_bmp)
set(L_IMAGE_IMAGEVIEW_TEST test_imageview)
set(L_IMAGE_FREETYPEWRAPPER_DEMO demo_freetypewrapper)

aux_source_directory(bmp src_bmp)
aux_source_directory(bmp/test src_bmp_test)
aux_source_directory(imageview src_imageview)
aux_source_directory(imageview/test src_imageview_test)
aux_source_directory(freetypewrapper src_freetypewrapper)
aux_source_directory(freetypewrapper/demo src_freetypewrapper_demo)

include_directories(
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${G_XTOOLS_XIO_HEADER_PATH}
    ${G_IMAGE_BMP_HEADER_PATH}
    ${G_IMAGE_IMAGEVIEW_HEADER_PATH}
    ${G_FREETYPE_INC_PATH}
    ${G_IMAGE_FREETYPEWRAPPER_HEADER_PATH}
    ${G_TEXTENC_UTF8_HEADER_PATH}
)

link_directories(
    ${G_FREETYPE_LIB_PATH}
)

add_library(${G_IMAGE} 
    SHARED 
    ${src_bmp}
    ${src_imageview}
    ${src_freetypewrapper})
target_link_libraries(${G_IMAGE}
    ${G_XTOOLS}
    freetyped
    ${G_TEXTENC})

if(${gnu})
set(COMMON_LIB gtest ${G_IMAGE} ${G_XTOOLS} ${G_TEXTENC} pthread)
elseif(${msvc})
set(COMMON_LIB gtest ${G_IMAGE} ${G_XTOOLS} ${G_TEXTENC})
elseif(${clang})
set(COMMON_LIB gtest ${G_IMAGE} ${G_XTOOLS} ${G_TEXTENC})
endif()

add_definitions(-DRES_BMP_PATH="${G_RESOURCE_PATH}/bmp")
add_definitions(-DRES_FONT_PATH="${G_RESOURCE_PATH}/font")

add_executable(${L_IMAGE_BMP_TEST}
    ${src_bmp_test}
    ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${L_IMAGE_BMP_TEST}
    ${COMMON_LIB})
add_test(NAME ${L_IMAGE_BMP_TEST}
        COMMAND ${L_IMAGE_BMP_TEST})

add_executable(${L_IMAGE_IMAGEVIEW_TEST}
    ${src_imageview_test}
    ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${L_IMAGE_IMAGEVIEW_TEST}
    ${COMMON_LIB})
add_test(NAME ${L_IMAGE_IMAGEVIEW_TEST}
        COMMAND ${L_IMAGE_IMAGEVIEW_TEST})

add_executable(${L_IMAGE_FREETYPEWRAPPER_DEMO}
    ${src_freetypewrapper_demo})
target_link_libraries(${L_IMAGE_FREETYPEWRAPPER_DEMO}
    ${COMMON_LIB}
    )