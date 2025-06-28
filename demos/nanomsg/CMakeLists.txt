aux_source_directory(client src_client)

aux_source_directory(server src_server)

set(DEMO_NAME demo_nn_client)
add_executable(${DEMO_NAME} ${src_client})
target_include_directories(${DEMO_NAME} 
    PRIVATE ${HEADER_NANOMSG}
    PRIVATE ${G_XTOOLS_XLOG_HEADER_PATH})
target_link_libraries(${DEMO_NAME} 
    ${LIB_NANOMSG} 
    ${G_XTOOLS})

set(DEMO_NAME demo_nn_server)
add_executable(${DEMO_NAME} ${src_server})
target_include_directories(${DEMO_NAME} 
    PRIVATE ${HEADER_NANOMSG}
    PRIVATE ${G_XTOOLS_XLOG_HEADER_PATH})
target_link_libraries(${DEMO_NAME} 
    ${LIB_NANOMSG} 
    ${G_XTOOLS})

