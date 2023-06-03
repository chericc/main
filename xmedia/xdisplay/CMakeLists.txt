
set(L_XDISPLAY xdisplay)

aux_source_directory(. src_xdisplay)

add_definitions(-DRES_VIDEO_PATH="${G_RESOURCE_PATH}/video")

include_directories(
	${G_XTOOLS_XTHREAD_HEADER_PATH}
	${G_XTOOLS_XUTILITY_HEADER_PATH}
)

link_directories(
	${G_SDL2_LIB_PATH}
	${G_FFMPEG_LIB_PATH}
)

add_executable(
	${L_XDISPLAY} WIN32
	${src_xdisplay}
)

target_include_directories(
	${L_XDISPLAY}
	PRIVATE ${G_XTOOLS_XLOG_HEADER_PATH}
	PRIVATE ${G_SDL2_INC_PATH}
	PRIVATE ${G_XPLAY_HEADER_PATH}
	PRIVATE ${G_FFMPEG_INC_PATH}
)

set(COMMON_LIB
	SDL2
	SDL2main
	${G_XPLAY}
	${G_XTOOLS})

target_link_libraries(
	${L_XDISPLAY}
	${COMMON_LIB}
)