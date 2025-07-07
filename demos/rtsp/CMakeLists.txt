
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

add_library(lib555_objects OBJECT ${src_live555})
set(common_options_no_warnings -Wno-all -Wno-extra -Wno-pedantic
    -Wno-int-to-pointer-cast -Wno-misleading-indentation -Wno-int-to-pointer-cast
    -Wno-implicit-fallthrough -Wno-format)

if (${clang})
set(common_options_no_warnings ${common_options_no_warnings} -Wno-deprecated-declarations
    -Wno-array-bounds)
target_compile_options(lib555_objects PRIVATE ${common_options_no_warnings})
else()
target_compile_options(lib555_objects PRIVATE ${common_options_no_warnings})
endif()

add_subdirectory(server)
add_subdirectory(client)