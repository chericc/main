set(NET_TEST_UTILS test_net_utils)
set(NET_TEST_TCPSOCKET test_net_tcp)

aux_source_directory(utils src_utils)
aux_source_directory(tcp src_tcp)

aux_source_directory(utils/test src_utils_test)
find_file(src_tcpsocket_test test_tcpsocket.cpp tcp/test)

include_directories(
    utils
    tcp
    ${G_XTOOLS_XLOG_HEADER_PATH}
)

add_library(${G_NET} SHARED
    ${src_utils}
    ${src_tcp}
)

if(${gnu})
set(COMMON_LIB gtest ${G_NET} ${G_XTOOLS} pthread)
elseif(${msvc})
set(COMMON_LIB gtest ${G_NET} ${G_XTOOLS})
elseif(${clang})
set(COMMON_LIB gtest ${G_NET} ${G_XTOOLS})
endif()

# test_net_utils
add_executable(${NET_TEST_UTILS}
    ${src_utils_test}
    ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${NET_TEST_UTILS} ${COMMON_LIB})
add_test(NAME ${NET_TEST_UTILS}
        COMMAND ${NET_TEST_UTILS})


# test_tcpsocket
add_executable(${NET_TEST_TCPSOCKET}
    ${src_tcpsocket_test}
    ${G_GOOGLETEST_MAIN_SRCFILE})
target_link_libraries(${NET_TEST_TCPSOCKET} ${COMMON_LIB})
add_test(NAME ${NET_TEST_TCPSOCKET}
        COMMAND ${NET_TEST_TCPSOCKET})