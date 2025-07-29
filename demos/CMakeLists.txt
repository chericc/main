
if(LIB_CJSON)
add_subdirectory(cjson)
endif()

if(LIB_C_ARES)
add_subdirectory(c-ares)
endif()

if(LIB_CURL)
add_subdirectory(curl)
endif()

add_subdirectory(ffmpeg)
if(${gnu})
add_subdirectory(h264_sei)
endif()
add_subdirectory(json_nlohmann)
add_subdirectory(mjson)

if(LIB_EVENT)
add_subdirectory(libevent)
endif()

if(LIB_LVGL)
add_subdirectory(lvgl/demo)
endif()

if(${gnu})
add_subdirectory(my_cond)
endif()

if(LIB_NANOMSG)
add_subdirectory(nanomsg)
endif()

add_subdirectory(packet_stable)

if(PATH_LIB_LIVE555)
add_subdirectory(rtsp)
endif()

if(LIB_SQLITE3)
add_subdirectory(sqlite)
endif()

if(${gnu} AND HEADER_TINYFRAME)
add_subdirectory(tinyframe)
add_subdirectory(uart)
endif()

if(LIB_WEBSOCKETS)
add_subdirectory(websockets)
endif()

if(${msvc})
add_subdirectory(winusb)
endif()

add_subdirectory(xtea)