#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include "SDL_Context.h"

namespace Assets {
	struct Sheet
	{
		Sheet();

		static SDL_Texture* texture;
	};
}