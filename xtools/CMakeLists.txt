
set(XTOOL_TEST_XLOG test_xlog)
set(XTOOL_TEST_XCONFIG test_xconfig)
set(XTOOL_TEST_XSTRING test_xstring)
set(XTOOL_TEST_XLOG_GTEST test_xlog_gtest)

aux_source_directory(xconfig src_xconfig)
aux_source_directory(xlog src_xlog)
aux_source_directory(xstring src_xstring)

aux_source_directory(xconfig/test src_xconfig_test)
aux_source_directory(xlog/test src_xlog_test)
aux_source_directory(xlog/test_gtest src_xlog_gtest)
aux_source_directory(xstring/test src_xstring_test)

include_directories(
        xconfig
        xlog
        xstring
        ${G_GOOGLETEST_INC_PATH}
)

link_directories(${GTEST_PATH}/lib)

add_library(${G_XTOOLS} SHARED ${src_xlog} ${src_xconfig} ${src_xstring})

if(${gnu})
set(COMMON_LIB gtest ${G_XTOOLS} pthread)
elseif(${msvc})
set(COMMON_LIB gtest ${G_XTOOLS})
endif()

add_executable(${XTOOL_TEST_XLOG} 
        ${src_xlog_test})
target_link_libraries(${XTOOL_TEST_XLOG} ${COMMON_LIB})

add_executable(${XTOOL_TEST_XCONFIG} 
        ${src_xconfig_test}
        ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${XTOOL_TEST_XCONFIG} ${COMMON_LIB})

add_executable(${XTOOL_TEST_XSTRING} 
        ${src_xstring_test}
        ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${XTOOL_TEST_XSTRING} ${COMMON_LIB})

add_executable(${XTOOL_TEST_XLOG_GTEST} 
        ${src_xlog_gtest}
        ${G_GOOGLETEST_MAIN_SRCFILE}
)
target_link_libraries(${XTOOL_TEST_XLOG_GTEST} ${COMMON_LIB})