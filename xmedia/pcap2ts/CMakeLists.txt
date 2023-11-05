set(L_PCAP2TS_DEMO demo_pcap2ts)

aux_source_directory(. src_pcap2ts)
aux_source_directory(demo src_pcap2ts_demo)

include_directories(
    ${G_XTOOLS_XIO_HEADER_PATH}
    ${G_XTOOLS_XUTILITY_HEADER_PATH}
    ${G_XTOOLS_XLOG_HEADER_PATH}
)

add_library(
    ${G_PCAP2TS} SHARED ${src_pcap2ts}
)

add_executable(
    ${L_PCAP2TS_DEMO}
    ${src_pcap2ts_demo}
)

target_link_libraries(
    ${L_PCAP2TS_DEMO} 
    ${G_XTOOLS}
    ${G_PCAP2TS}
)
target_include_directories(
    ${L_PCAP2TS_DEMO} 
    PRIVATE ${G_PCAP2TS_HEADER_PATH}
)