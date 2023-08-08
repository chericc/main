
set(XTOOL_TEST_XLOG_DEMO demo_xtools_xlog)
set(XTOOL_TEST_XLOG_TEST test_xtools_xlog)
set(XTOOL_TEST_XCONFIG test_xtools_xconfig)
set(XTOOL_TEST_XSTRING test_xtools_xstring)
set(XTOOL_TEST_XIO test_xtools_xio)
set(XTOOL_TEST_XTHREAD test_xtools_xthread)
set(XTOOL_DEMO_XTIMER demo_xtools_xtimer)

aux_source_directory(xconfig src_xconfig)
aux_source_directory(xlog src_xlog)
aux_source_directory(xstring src_xstring)
aux_source_directory(xio src_xio)
aux_source_directory(xthread src_xthread)
aux_source_directory(xtimer src_xtimer)

aux_source_directory(xconfig/test src_xconfig_test)
aux_source_directory(xlog/demo src_xlog_demo)
aux_source_directory(xlog/test_gtest src_xlog_test)
aux_source_directory(xstring/test src_xstring_test)
aux_source_directory(xio/test src_xio_test)
aux_source_directory(xthread/test src_xthread_test)
aux_source_directory(xtimer/demo src_xtimer_demo)

include_directories(
        xconfig
        xlog
        xstring
        xio
        xthread
        xutility
        xtimer
        ${G_GOOGLETEST_INC_PATH}
)

add_library(${G_XTOOLS} SHARED ${src_xlog} ${src_xconfig} ${src_xstring} ${src_xio} ${src_xthread} ${src_xtimer})

# tests

if(${gnu})
set(COMMON_LIB gtest ${G_XTOOLS} pthread)
elseif(${msvc})
set(COMMON_LIB gtest ${G_XTOOLS})
elseif(${clang})
set(COMMON_LIB gtest ${G_XTOOLS})
endif()

# demo_xlog
add_executable(${XTOOL_TEST_XLOG_DEMO} 
        ${src_xlog_demo})
target_link_libraries(${XTOOL_TEST_XLOG_DEMO} ${COMMON_LIB})

# test_xthread
add_executable(${XTOOL_TEST_XTHREAD} 
        ${src_xthread_test}
        ${G_GOOGLETEST_MAIN_SRCFILE}
)
target_link_libraries(${XTOOL_TEST_XTHREAD} ${COMMON_LIB})
add_test(NAME ${XTOOL_TEST_XTHREAD}
        COMMAND ${XTOOL_TEST_XTHREAD})

# test_xlog
add_executable(${XTOOL_TEST_XLOG_TEST} 
        ${src_xlog_test}
        ${G_GOOGLETEST_MAIN_SRCFILE}
)
target_link_libraries(${XTOOL_TEST_XLOG_TEST} ${COMMON_LIB})
add_test(NAME ${XTOOL_TEST_XLOG_TEST}
        COMMAND ${XTOOL_TEST_XLOG_TEST})

# test_xconfig
add_executable(${XTOOL_TEST_XCONFIG} 
        ${src_xconfig_test}
        ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${XTOOL_TEST_XCONFIG} ${COMMON_LIB})
add_test(NAME ${XTOOL_TEST_XCONFIG}
        COMMAND ${XTOOL_TEST_XCONFIG})

# test_xstring
add_executable(${XTOOL_TEST_XSTRING} 
        ${src_xstring_test}
        ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${XTOOL_TEST_XSTRING} ${COMMON_LIB})
add_test(NAME ${XTOOL_TEST_XSTRING}
        COMMAND ${XTOOL_TEST_XSTRING})

# test_xio
add_executable(${XTOOL_TEST_XIO} 
        ${src_xio_test}
        ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${XTOOL_TEST_XIO} ${COMMON_LIB})
add_test(NAME ${XTOOL_TEST_XIO}
        COMMAND ${XTOOL_TEST_XIO})

# demo_xtimer
add_executable(${XTOOL_DEMO_XTIMER} 
        ${src_xtimer_demo})
target_link_libraries(${XTOOL_DEMO_XTIMER} ${COMMON_LIB})