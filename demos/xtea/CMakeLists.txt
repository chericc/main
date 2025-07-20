set(XTEA_TEST test_xtea)
set(DEMO_NAME demo_xtea)

aux_source_directory(. src)
aux_source_directory(./demo src_demo)


add_executable(${DEMO_NAME} 
    ${src} 
    ${src_demo}
)

target_link_libraries(
    ${DEMO_NAME}
    ${G_XTOOLS}
)

include_directories(.)

target_include_directories(
    ${DEMO_NAME}
    PRIVATE ${G_XTOOLS_XLOG_HEADER_PATH}
)

if(LIB_GTEST)

include_directories(${HEADER_GTEST} ${G_XTOOLS_XLOG_HEADER_PATH})
link_libraries(
    ${G_XTOOLS}
)

set(TEST_LIB ${LIB_GTEST} ${G_TEST_COMM})

aux_source_directory(test src_test)
add_executable(${XTEA_TEST}
    ${src} 
    ${src_test}
    ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${XTEA_TEST} 
    ${TEST_LIB})
# target_include_directories(${XTEA_TEST}
    # PRIVATE ${G_XTOOLS_XLOG_HEADER_PATH}
# )
endif()