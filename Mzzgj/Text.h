#pragma once

#include <string>
#include <SDL.h>
#include <SDL_ttf.h>


struct Text
{
	Text(std::string text, int x, int y, int w, int h);

	Text(std::string text, int x, int y, int w, int h, SDL_Color color);

	Text(std::string text, int x, int y, int h);

	Text(std::string text, int x, int y, int h, SDL_Color color);

	~Text();

	void load_text();

	void load_text_and_rct(int x, int y, int h);

	void update(std::string text);

	void render();

	std::string text;
	SDL_Texture* texture;
	SDL_Rect rct;

	SDL_Color color;
};