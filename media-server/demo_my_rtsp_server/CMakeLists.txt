
aux_source_directory(. src)
set(BIN demo_my_rtsp_server)
add_executable(
    ${BIN}
    ${src}
)
target_compile_definitions(
    ${BIN}
    PRIVATE
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

