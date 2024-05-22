include_directories(${G_SDL2_INC_PATH} ${G_XTOOLS_XLOG_HEADER_PATH})
link_directories(${G_SDL2_LIB_PATH})

link_libraries(${G_SDL2} ${G_XTOOLS})

aux_source_directory(maze_game src)

set(PROGNAME sdl_maze_game)
add_executable(${PROGNAME} ${src})