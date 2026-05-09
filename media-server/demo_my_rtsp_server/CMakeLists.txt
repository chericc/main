
set(src
    main.cpp
    h264-file-reader.cpp
    h264-file-source.cpp
    h265-file-reader.cpp
    h265-file-source.cpp
    rtp-udp-transport.cpp
)
set(BIN demo_my_rtsp_server)
add_executable(
    ${BIN}
    ${src}
)
target_link_libraries(
    ${BIN}
    ${LIB_MEDIA_SERVER}
    ${G_XTOOLS}
)
install(
    TARGETS ${BIN}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
