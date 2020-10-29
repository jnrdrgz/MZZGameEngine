#pragma once

#include <SDL.h>
#include <string>

#include "SDL_Context.h"
#include "AssetsManager.h"
#include "Camera.h"

#include "components/ParticleEmitter.h"
#include "components/AI.h"
#include "components/InputHandler.h"
#include "components/Animation.h"


struct GameObject
{
	GameObject(int x, int y, int w, int h, int tilex, int tiley, std::string tag);

	void draw();

	void set_input_handler(std::unique_ptr<InputHandler> ptr);

	void set_ai(std::unique_ptr<AI> ptr);

	void set_animation(std::unique_ptr<Animation> ptr);

	void set_emitter(std::unique_ptr<ParticleEmitter> ptr);

	int tile;
	double angle;
	SDL_RendererFlip flip;
	SDL_Rect dst, src, dst_camera;
	std::string tag;
	bool active;

	std::unique_ptr<InputHandler> input_handler{ nullptr };
	std::unique_ptr<AI> ai{ nullptr };
	std::unique_ptr<Animation> animation{ nullptr };
	std::unique_ptr<ParticleEmitter> emitter{ nullptr };
	//updater()??

};