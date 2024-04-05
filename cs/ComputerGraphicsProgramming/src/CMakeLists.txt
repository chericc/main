cmake_minimum_required(VERSION 3.0)

project(opengl_demo)

set(CMAKE_CXX_STANDARD 14)

aux_source_directory(prog_2_1 src_prog_2_1)
aux_source_directory(prog_2_2 src_prog_2_2)
aux_source_directory(prog_2_3 src_prog_2_3)

set(PROG_2_1 prog_2_1)
set(PROG_2_2 prog_2_2)
set(PROG_2_3 prog_2_3)

include_directories(
	D:/opensrc/glew-2.1.0-win32/glew-2.1.0/include 
	D:/opensrc/glfw-3.4.bin.WIN64/include
	utility/error_info
)

link_directories(
	D:/opensrc/glew-2.1.0-win32/glew-2.1.0/lib/Release/x64
	D:/opensrc/glfw-3.4.bin.WIN64/lib-vc2022)

link_libraries(
	glfw3dll GLEW32 OpenGL32
)

add_executable(${PROG_2_1}
	${src_prog_2_1})

add_executable(${PROG_2_2}
	${src_prog_2_2})
	
add_executable(${PROG_2_3}
	${src_prog_2_3})