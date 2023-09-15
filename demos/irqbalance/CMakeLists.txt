cmake_minimum_required(VERSION 3.0)

project(irqbalance)

set(BIN_IRQBALANCE iRqbalance)
set(PATH_CONFIG ../build)

set(SRCS_IRQBALANCE 
    ${CMAKE_SOURCE_DIR}/activate.c 
    ${CMAKE_SOURCE_DIR}/bitmap.c
    ${CMAKE_SOURCE_DIR}/classify.c
    ${CMAKE_SOURCE_DIR}/cputree.c
    ${CMAKE_SOURCE_DIR}/irqbalance.c
    ${CMAKE_SOURCE_DIR}/irqlist.c
    ${CMAKE_SOURCE_DIR}/numa.c
    ${CMAKE_SOURCE_DIR}/placement.c
    ${CMAKE_SOURCE_DIR}/procinterrupts.c
    # ${CMAKE_SOURCE_DIR}/thermal.c
)

add_executable(${BIN_IRQBALANCE}
    ${SRCS_IRQBALANCE})

target_include_directories(${BIN_IRQBALANCE} 
    PRIVATE ${PATH_CONFIG}
    PRIVATE /usr/include/glib-2.0 
    PRIVATE /usr/lib/x86_64-linux-gnu/glib-2.0/include)

target_link_libraries(${BIN_IRQBALANCE} 
    glib-2.0
    m
)