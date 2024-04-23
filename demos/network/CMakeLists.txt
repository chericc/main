set(NETWORK_LIBEVENT_DEMO_GETADDRINFO demo_network_libevent_getaddrinfo)
set(NETWORK_LIBEVENT_DEMO_ECHOSERVER demo_network_libevent_echoserver)

aux_source_directory(libevent/getaddrinfo src_demo_libevent_getaddrinfo)
aux_source_directory(libevent/echo_server src_demo_libevent_echoserver)

if (WITH_LIBEVENT)

include_directories(${G_XTOOLS_XLOG_HEADER_PATH}
                    ${G_XTOOLS_XCROSSPLATFORM_HEADER_PATH}
                    ${G_LIBEVENT_INC_PATH})
link_directories(${G_LIBEVENT_LIB_PATH})
link_libraries(${G_XTOOLS} ${G_LIBEVENT})

add_executable(${NETWORK_LIBEVENT_DEMO_GETADDRINFO}
    ${src_demo_libevent_getaddrinfo})

add_executable(${NETWORK_LIBEVENT_DEMO_ECHOSERVER}
    ${src_demo_libevent_echoserver})

endif()