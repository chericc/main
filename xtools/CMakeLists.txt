
set(XTOOL_TEST_XLOG demo_xtools_xlog)
set(XTOOL_TEST_XLOG_GTEST test_xtools_xlog_gtest)
set(XTOOL_TEST_XCONFIG test_xtools_xconfig)
set(XTOOL_TEST_XSTRING test_xtools_xstring)
set(XTOOL_TEST_XIO test_xtools_xio)

aux_source_directory(xconfig src_xconfig)
aux_source_directory(xlog src_xlog)
aux_source_directory(xstring src_xstring)
aux_source_directory(xio src_xio)

aux_source_directory(xconfig/test src_xconfig_test)
aux_source_directory(xlog/test src_xlog_test)
aux_source_directory(xlog/test_gtest src_xlog_gtest)
aux_source_directory(xstring/test src_xstring_test)
aux_source_directory(xio/test src_xio_test)

include_directories(
        xconfig
        xlog
        xstring
        xio
        ${G_GOOGLETEST_INC_PATH}
)

add_library(${G_XTOOLS} SHARED ${src_xlog} ${src_xconfig} ${src_xstring} ${src_xio})

# tests

if(${gnu})
set(COMMON_LIB gtest ${G_XTOOLS} pthread)
elseif(${msvc})
set(COMMON_LIB gtest ${G_XTOOLS})
endif()

add_executable(${XTOOL_TEST_XLOG} 
        ${src_xlog_test})
target_link_libraries(${XTOOL_TEST_XLOG} ${COMMON_LIB})

add_executable(${XTOOL_TEST_XLOG_GTEST} 
        ${src_xlog_gtest}
        ${G_GOOGLETEST_MAIN_SRCFILE}
)
target_link_libraries(${XTOOL_TEST_XLOG_GTEST} ${COMMON_LIB})
add_test(NAME ${XTOOL_TEST_XLOG_GTEST}
        COMMAND ${XTOOL_TEST_XLOG_GTEST})

add_executable(${XTOOL_TEST_XCONFIG} 
        ${src_xconfig_test}
        ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${XTOOL_TEST_XCONFIG} ${COMMON_LIB})
add_test(NAME ${XTOOL_TEST_XCONFIG}
        COMMAND ${XTOOL_TEST_XCONFIG})

add_executable(${XTOOL_TEST_XSTRING} 
        ${src_xstring_test}
        ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${XTOOL_TEST_XSTRING} ${COMMON_LIB})
add_test(NAME ${XTOOL_TEST_XSTRING}
        COMMAND ${XTOOL_TEST_XSTRING})

add_executable(${XTOOL_TEST_XIO} 
        ${src_xio_test}
        ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${XTOOL_TEST_XIO} ${COMMON_LIB})
add_test(NAME ${XTOOL_TEST_XIO}
        COMMAND ${XTOOL_TEST_XIO})