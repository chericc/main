#include <xlog.hpp>

#include <SDL.h>

int main(int argc, char *argv[])
{
	xlog_dbg("Hello world!");

	do
	{
		int flags = SDL_INIT_VIDEO;

		if (SDL_Init(flags)) {
			xlog_err("SDL_Init failed");
			break;
		}

		flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
		SDL_Window* window = SDL_CreateWindow("test window",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			800, 600, flags);

		if (!window) {
			xlog_err("SDL_CreateWindow failed");
			break;
		}

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

		while (true) {
			SDL_Event event;
			while (!SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
				xlog_dbg("pump events");
				SDL_PumpEvents();
			}

			if (event.type == SDL_QUIT) {
				break;
			}
		}
	} while (0);

	return 0;
}