#

set(FDKAAC_DEMO demo_fdkaac_wrapper)

aux_source_directory(. src_fdkaac_wrapper)
aux_source_directory(demo src_fdkaac_wrapper_demo)

include_directories(
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${G_FDKAAC_INC_PATH}
    ${G_FDKAAC_WRAPPER_HEADER_PATH}
)

add_library(${G_FDKAAC_WRAPPER}
    ${src_fdkaac_wrapper})
target_link_libraries(${G_FDKAAC_WRAPPER}
    ${G_XTOOLS})

link_directories(${G_FDKAAC_LIB_PATH})

set(COMMON_LIB ${G_FDKAAC_WRAPPER} fdk-aac ${G_XTOOLS})

add_executable(${FDKAAC_DEMO}
    ${src_fdkaac_wrapper_demo})
target_link_libraries(${FDKAAC_DEMO}
    ${COMMON_LIB})
