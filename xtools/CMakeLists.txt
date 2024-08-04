
set(XTOOL_TEST_XLOG_DEMO demo_xtools_xlog)
set(XTOOL_TEST_XLOG_TEST test_xtools_xlog)
set(XTOOL_TEST_XCONFIG test_xtools_xconfig)
set(XTOOL_TEST_XSTRING test_xtools_xstring)
set(XTOOL_TEST_XIO test_xtools_xio)
set(XTOOL_TEST_XTHREAD test_xtools_xthread)
set(XTOOL_TEST_XTHREADPOOL test_xtools_xthreadpool)
set(XTOOL_DEMO_XTIMER demo_xtools_xtimer)
set(XTOOL_DEMO_XTEST_CLOCK demo_xtools_xtest_clock)
set(XTOOL_TEST_XTEST_CLOCK test_xtools_xtest_clock)
set(XTOOL_TEST_XUTILITY_XBSWAP test_xtools_xutility_xbswap)
set(XTOOL_TEST_XUTILITY_XPERFTEST test_xtools_xutility_xperftest)

aux_source_directory(xconfig src_xconfig)
aux_source_directory(xcrossplatform src_xcrossplatform)
aux_source_directory(xlog src_xlog)
aux_source_directory(xstring src_xstring)
aux_source_directory(xio src_xio)
aux_source_directory(xthread src_xthread)
aux_source_directory(xthreadpool src_xthreadpool)
aux_source_directory(xtimer src_xtimer)
aux_source_directory(xtest src_xtest)
aux_source_directory(xutility src_xutility)

aux_source_directory(xconfig/test src_xconfig_test)
aux_source_directory(xlog/demo src_xlog_demo)
aux_source_directory(xlog/test_gtest src_xlog_test)
aux_source_directory(xstring/test src_xstring_test)
aux_source_directory(xio/test src_xio_test)
aux_source_directory(xthread/test src_xthread_test)
aux_source_directory(xthreadpool/test src_xthreadpool_test)
aux_source_directory(xtimer/demo src_xtimer_demo)
aux_source_directory(xtest/demo_xtest_clock src_xtest_clock_demo)
aux_source_directory(xtest/test_xtest_clock src_xtest_clock_test)
aux_source_directory(xutility/test_bswap src_xutility_test_bswap)
aux_source_directory(xutility/test_xperftest src_xutility_test_xperftest)

include_directories(
        xconfig
        xcrossplatform
        xlog
        xstring
        xio
        xthread
        xthreadpool
        xutility
        xtimer
        xtest
        ${G_GOOGLETEST_INC_PATH}
)

add_library(${G_XTOOLS} SHARED 
        ${src_xlog} 
        ${src_xconfig} 
        ${src_xcrossplatform}
        ${src_xstring} 
        ${src_xio} 
        ${src_xthread} 
        ${src_xthreadpool}
        ${src_xtimer}
        ${src_xtest}
        ${src_xutility})

# tests

if(WITH_GTEST)
set(TEST_LIB ${G_GOOGLETEST} ${G_TEST_COMM})
endif()

if(${gnu})
set(COMMON_LIB ${TEST_LIB} ${G_XTOOLS} pthread)
elseif(${msvc})
set(COMMON_LIB ${TEST_LIB} ${G_XTOOLS})
elseif(${clang})
set(COMMON_LIB ${TEST_LIB} ${G_XTOOLS})
endif()

# demo_xlog
add_executable(${XTOOL_TEST_XLOG_DEMO} 
        ${src_xlog_demo})
target_link_libraries(${XTOOL_TEST_XLOG_DEMO} ${COMMON_LIB})

if(WITH_GTEST)
# test_xthread
add_executable(${XTOOL_TEST_XTHREAD} 
        ${src_xthread_test}
        ${G_GOOGLETEST_MAIN_SRCFILE}
)
target_link_libraries(${XTOOL_TEST_XTHREAD} ${COMMON_LIB})
add_test(NAME ${XTOOL_TEST_XTHREAD}
        COMMAND ${XTOOL_TEST_XTHREAD})

# test_xthreadpool
add_executable(${XTOOL_TEST_XTHREADPOOL} 
        ${src_xthreadpool_test}
        ${G_GOOGLETEST_MAIN_SRCFILE}
)
target_link_libraries(${XTOOL_TEST_XTHREADPOOL} ${COMMON_LIB})
add_test(NAME ${XTOOL_TEST_XTHREADPOOL}
        COMMAND ${XTOOL_TEST_XTHREADPOOL})

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
target_include_directories(${XTOOL_TEST_XIO} 
        PRIVATE ${G_TSET_COMM_HEADER_PATH})
add_test(NAME ${XTOOL_TEST_XIO}
        COMMAND ${XTOOL_TEST_XIO})

add_executable(${XTOOL_TEST_XTEST_CLOCK}
        ${src_xtest_clock_test}
        ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${XTOOL_TEST_XTEST_CLOCK} ${COMMON_LIB})
add_test(NAME ${XTOOL_TEST_XTEST_CLOCK}
        COMMAND ${XTOOL_TEST_XTEST_CLOCK})

# test_xtools_xutility_xbswap
add_executable(${XTOOL_TEST_XUTILITY_XBSWAP} 
        ${src_xutility_test_bswap}
        ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${XTOOL_TEST_XUTILITY_XBSWAP} ${COMMON_LIB})
add_test(NAME ${XTOOL_TEST_XUTILITY_XBSWAP}
        COMMAND ${XTOOL_TEST_XUTILITY_XBSWAP})

# test_xtools_xutility_xperftest
add_executable(${XTOOL_TEST_XUTILITY_XPERFTEST} 
        ${src_xutility_test_xperftest}
        ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${XTOOL_TEST_XUTILITY_XPERFTEST} ${COMMON_LIB})
add_test(NAME ${XTOOL_TEST_XUTILITY_XPERFTEST}
        COMMAND ${XTOOL_TEST_XUTILITY_XPERFTEST})
endif()

# demo_xtimer
add_executable(${XTOOL_DEMO_XTIMER} 
        ${src_xtimer_demo})
target_link_libraries(${XTOOL_DEMO_XTIMER} ${COMMON_LIB})

# demo_xtest_clock
add_executable(${XTOOL_DEMO_XTEST_CLOCK}
        ${src_xtest_clock_demo})
target_link_libraries(${XTOOL_DEMO_XTEST_CLOCK} ${COMMON_LIB})