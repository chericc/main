
add_subdirectory(pcap2ts)

if(LIB_FFMPEG_AVFORMAT
  AND LIB_FFMPEG_AVCODEC
  AND LIB_FFMPEG_AVUTIL
  AND LIB_FFMPEG_SWRESAMPLE
  AND LIB_FFMPEG_SWSCALE)
add_subdirectory(xplay)
add_subdirectory(myffmpeg)
if(LIB_SDL2)
add_subdirectory(xdisplay)
endif() # sdl
endif() # ffmpeg
