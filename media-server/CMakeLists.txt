set(LIB_MEDIA_SERVER media_server)
set(DEMO_RTSP_CLIENT_INNER demo_rtsp_client_inner)
set(DEMO_RTSP_CLIENT demo_rtsp_client)

add_definitions(-DOS_LINUX)

if(NOT CMAKE_BUILD_TYPE)
    message(CRITICAL "CMAKE_BUILD_TYPE not set")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DDEBUG -D_DEBUG)
else()
    # add_definitions(-DNDEBUG) # this is automatically added by cmake in release mode
endif()

add_compile_options(
    -Wno-unused-variable 
    -Wno-unused-const-variable
    -Wno-pedantic
    -Wno-sign-compare
    -Wno-type-limits
    -Wno-unused-function
    -Wno-format-truncation
    -Wno-unused-but-set-variable
    -Wno-unused-value
    -Wno-unused-result
)

aux_source_directory(libbase/source/digest src_base_digest)
aux_source_directory(libbase/source/i18n src_base_i18n)
aux_source_directory(libbase/source/port src_base_port)
aux_source_directory(libbase/source/string src_base_string)
aux_source_directory(libbase/source/ src_base_root)
set(src_base ${src_base_digest} ${src_base_i18n} ${src_base_port} ${src_base_string} ${src_base_root})

# libflv
aux_source_directory(libflv/source src_flv_root)
set(src_flv ${src_flv_root})

# libhttp
aux_source_directory(libhttp/source src_http_root)
set(src_http ${src_http_root})

# libaio
aux_source_directory(libaio/src src_aio_root)
set(src_aio ${src_aio_root})

# librtsp
aux_source_directory(librtsp/source/ src_rtsp_root)
aux_source_directory(librtsp/source/client src_rtsp_client)
aux_source_directory(librtsp/source/sdp src_rtsp_rdp)
aux_source_directory(librtsp/source/server src_rtsp_utils)
set(src_rtsp ${src_rtsp_root} ${src_rtsp_client} ${src_rtsp_rdp} ${src_rtsp_utils})

# librtp
aux_source_directory(librtp/source/ src_rtp_source)
aux_source_directory(librtp/payload/ src_rtp_payload)
aux_source_directory(librtp/rtpext src_rtp_rtpext)
set(src_rtp ${src_rtp_source} ${src_rtp_payload} ${src_rtp_rtpext})

# libmedia_server
set(src_media_server ${src_base} ${src_flv} ${src_http} ${src_aio} ${src_rtsp} ${src_rtp})

add_library(${LIB_MEDIA_SERVER} ${src_media_server})

# demo
include_directories(librtp/include)
include_directories(librtsp/include)
include_directories(libbase/include)
include_directories(libhttp/include)
include_directories(libflv/include)
include_directories(libaio/include)
include_directories(${G_XTOOLS_XLOG_HEADER_PATH})

aux_source_directory(demo_rtsp_client src_demo_rtsp_client)
add_executable(${DEMO_RTSP_CLIENT_INNER}
    ${src_demo_rtsp_client})
target_link_libraries(
    ${DEMO_RTSP_CLIENT_INNER}
    ${LIB_MEDIA_SERVER}
    ${G_XTOOLS}
)

aux_source_directory(rtsp_client src_rtsp_client)
add_executable(${DEMO_RTSP_CLIENT}
    ${src_rtsp_client})
target_link_libraries(
    ${DEMO_RTSP_CLIENT}
    ${LIB_MEDIA_SERVER}
    ${G_XTOOLS}
)
