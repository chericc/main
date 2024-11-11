set(NETWORK_LIBEVENT_DEMO_GETADDRINFO demo_network_libevent_getaddrinfo)
set(NETWORK_LIBEVENT_DEMO_ECHOSERVER demo_network_libevent_echoserver)
set(NETWORK_LIBEVENT_DEMO_HTTPSERVER demo_network_libevent_httpserver)

aux_source_directory(src/getaddrinfo src_demo_libevent_getaddrinfo)
aux_source_directory(src/echo_server src_demo_libevent_echoserver)
aux_source_directory(src/httpserver src_demo_libevent_httpserver)

include_directories(${G_XTOOLS_XLOG_HEADER_PATH}
                    ${G_XTOOLS_XCROSSPLATFORM_HEADER_PATH})
link_libraries(${G_XTOOLS} ${LIB_EVENT})

add_executable(${NETWORK_LIBEVENT_DEMO_GETADDRINFO}
    ${src_demo_libevent_getaddrinfo})

add_executable(${NETWORK_LIBEVENT_DEMO_ECHOSERVER}
    ${src_demo_libevent_echoserver})
    
add_executable(${NETWORK_LIBEVENT_DEMO_HTTPSERVER}
    ${src_demo_libevent_httpserver})
