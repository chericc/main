
add_definitions(-DNO_OPENSSL=1 -DSOCKLEN_T=socklen_t -DNO_OPENSSL=1 -DNO_STD_LIB=1)

include_directories(
    ${HEADER_LIVE555}
    ${HEADER_LIVE555}/BasicUsageEnvironment
    ${HEADER_LIVE555}/groupsock
    ${HEADER_LIVE555}/liveMedia
    ${HEADER_LIVE555}/UsageEnvironment
)
link_directories(
    ${LIB_LIVE555_PATH}
)
link_libraries(
    ${LIB_LIVE555}
)

add_subdirectory(server)
add_subdirectory(client)