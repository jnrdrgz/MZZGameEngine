#pragma once

#include <SDL.h>

struct GameObject;

struct InputHandler
{
	virtual void handle_input(GameObject& gameobject) = 0;

	bool pressed[512]{ false };
	SDL_Scancode last_scancode;
};