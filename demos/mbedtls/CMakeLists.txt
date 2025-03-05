cmake_minimum_required(VERSION 3.00)
project(main)

set(MBEDTLS_PATH ext_lib/mbedtls)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

aux_source_directory(client src_client)
aux_source_directory(${MBEDTLS_PATH}/library src_mbedtls)

include_directories(${MBEDTLS_PATH}/include)

add_executable(demo_mbedtls_client
    ${src_client}
    ${src_mbedtls})