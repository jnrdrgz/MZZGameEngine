#pragma once

#include <SDL.h>
#include <string>
#include <iostream>

#include "SDL_Context.h"
#include "Text.h"

struct Button
{
	Button(int x, int y, int w, int h, std::string text, int margin) :
		rct{ x,y,w,h }, text{ text,x + margin,y + margin,w - margin * 2,h - margin * 2 },
		text_str{ text },
		selected{ false },
		color{ 255,0,0,255 }
	{
		std::cout << "buttoin created: " << x << "-" << y << "-" << w << "-" << h << "-" << margin << "\n";

	}

	void draw() {
		SDL_SetRenderDrawColor(SDL::Context::renderer, color.r, color.g, color.b, color.a);
		SDL_RenderFillRect(SDL::Context::renderer, &rct);
		SDL_SetRenderDrawColor(SDL::Context::renderer, 255, 255, 255, 255);
		text.render();
	}

	bool handle_input() {
		mx = SDL::Context::event.button.x;
		my = SDL::Context::event.button.y;

		//std::cout << x << "\n";
		if (mx > rct.x && mx<rct.x + rct.w
			&& my > rct.y && my < rct.y + rct.h) {

			color.r = 0;
			if (SDL_GetMouseState(&mx, &my) && SDL_BUTTON(SDL_BUTTON_LEFT)) return true;
		}
		else {
			color.r = 255;
		}


		return false;
	}

	SDL_Rect rct;
	std::string text_str;
	char tag;
	Text text;
	bool selected;
	int mx{ 0 }, my{ 0 };
	SDL_Color color;
};
