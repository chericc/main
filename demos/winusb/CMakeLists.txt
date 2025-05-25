aux_source_directory(. src)
add_library(my_win_usb ${src})

aux_source_directory(demo src_demo)

include_directories(. ${G_XTOOLS_XLOG_HEADER_PATH})

add_executable(demo_win_usb ${src_demo})
target_link_libraries(demo_win_usb my_win_usb Setupapi wbemuuid ${G_XTOOLS})
