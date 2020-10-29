#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>

namespace SDL {
	struct Context
	{
	public:
		Context(const char* title, int screen_w, int screen_h);

		~Context();

		static SDL_Renderer* renderer;
		static TTF_Font* font;
		static SDL_Event event;
		bool running = false;

	private:
		SDL_Window* window = NULL;
	};
}
