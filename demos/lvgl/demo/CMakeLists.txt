set(DEMO_LVGL demo_lvgl)
aux_source_directory(. src)

add_compile_definitions(LV_CONF_PATH="lvgl/lv_conf.h")

include_directories(${G_XTOOLS_XLOG_HEADER_PATH} ${HEADER_LVGL})

add_executable(${DEMO_LVGL} ${src})

target_link_libraries(${DEMO_LVGL} 
    ${G_XTOOLS} 
    ${LIB_LVGL} 
    ${LIB_LVGL_THORVG}
    ${LIB_SDL2} 
)