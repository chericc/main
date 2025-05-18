
add_definitions(-DNO_OPENSSL=1 -DSOCKLEN_T=socklen_t -DNO_OPENSSL=1 -DNO_STD_LIB=1)

include_directories(
    ${PATH_LIB_LIVE555}/groupsock/include
    ${PATH_LIB_LIVE555}/liveMedia/include
    ${PATH_LIB_LIVE555}/UsageEnvironment/include
    ${PATH_LIB_LIVE555}/BasicUsageEnvironment/include)

aux_source_directory(${PATH_LIB_LIVE555}/groupsock src_live555)
aux_source_directory(${PATH_LIB_LIVE555}/liveMedia src_live555)
aux_source_directory(${PATH_LIB_LIVE555}/UsageEnvironment src_live555)
aux_source_directory(${PATH_LIB_LIVE555}/BasicUsageEnvironment src_live555)

add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(live555wrap)