set(NETWORK_LIBEVENT_DEMO demo_network_libevent)

aux_source_directory(libevent src_demo_libevent)

if (HAS_LIB_EVENT)

add_executable(${NETWORK_LIBEVENT_DEMO}
    ${src_demo_libevent})
target_include_directories(${NETWORK_LIBEVENT_DEMO}
    PRIVATE ${G_XTOOLS_XLOG_HEADER_PATH}
    PRIVATE ${G_LIBEVENT_INC_PATH})
target_link_directories(${NETWORK_LIBEVENT_DEMO}
    PRIVATE ${G_LIBEVENT_LIB_PATH})
target_link_libraries(${NETWORK_LIBEVENT_DEMO}
    xtools event)
endif()