set(LIB_NAME packet_stable)
aux_source_directory(. src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${G_XTOOLS_XLOG_HEADER_PATH})
link_libraries(${G_XTOOLS})

add_library(${LIB_NAME} ${src})

link_libraries(${LIB_NAME})

set(BIN_NAME demo_packet_stable)
aux_source_directory(./demo_packet_stable src_packet_stable)
add_executable(${BIN_NAME}
    ${src_packet_stable})

set(BIN_NAME demo_packet_gen)
aux_source_directory(./demo_packet_gen src_packet_gen)
add_executable(${BIN_NAME}
    ${src_packet_gen})