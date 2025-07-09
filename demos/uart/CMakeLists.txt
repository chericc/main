aux_source_directory(${HEADER_TINYFRAME} src_tinyframe_lib)

aux_source_directory(. src)

include_directories(.)

set(DEMO_LIB my_uart)
add_library(${DEMO_LIB}
    ${src}
)
target_include_directories(${DEMO_LIB} 
    PRIVATE ${G_XTOOLS_XLOG_HEADER_PATH}
    PRIVATE ${HEADER_TINYFRAME}
)

aux_source_directory(demo_uart_raw src_demo_usrt_raw)
set(DEMO demo_uart_raw)
add_executable(${DEMO} ${src_demo_usrt_raw})
target_link_libraries(${DEMO} 
    ${DEMO_LIB} ${G_XTOOLS}
)
target_include_directories(${DEMO} 
    PRIVATE ${G_XTOOLS_XLOG_HEADER_PATH}
)