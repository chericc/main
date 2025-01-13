
add_definitions(-DNO_OPENSSL=1)

include_directories(
    ${HEADER_LIB_LIVE555}/BasicUsageEnvironment
    ${HEADER_LIB_LIVE555}/groupsock
    ${HEADER_LIB_LIVE555}/liveMedia
    ${HEADER_LIB_LIVE555}/UsageEnvironment)

link_libraries(
    ${LIB_LIVE555_liveMedia}
    ${LIB_LIVE555_groupsock}
    ${LIB_LIVE555_UsageEnvironment}
    ${LIB_LIVE555_BasicUsageEnvironment})

add_subdirectory(server)