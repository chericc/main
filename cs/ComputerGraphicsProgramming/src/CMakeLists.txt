cmake_minimum_required(VERSION 3.0)

project(opengl_demo)

set(CMAKE_CXX_STANDARD 14)

aux_source_directory(prog_2_1_1 src_prog_2_1_1)

set(PROG_2_1_1 prog_2_1_1)

include_directories(
	D:/opensrc/glew-2.1.0-win32/glew-2.1.0/include 
	D:/opensrc/glfw-3.4.bin.WIN64/include
)

add_executable(${PROG_2_1_1}
	${src_prog_2_1_1})
target_link_directories(${PROG_2_1_1}
	PRIVATE D:/opensrc/glew-2.1.0-win32/glew-2.1.0/lib/Release/x64
	PRIVATE D:/opensrc/glfw-3.4.bin.WIN64/lib-vc2022)
target_link_libraries(${PROG_2_1_1}
	glfw3dll GLEW32 OpenGL32)