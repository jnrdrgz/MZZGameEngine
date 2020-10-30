#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include "demo/demo1.h"

int main(int argc, char* args[])
{
	(void)argc;
	(void)args;

	SDL::Context sdl_context("game", 640, 480);
	Assets::Sheet sheet;
	bool running = true;

	Uint32 startFTime = SDL_GetTicks();
	float fps = (1.0f / 24.0f)*1000.0f;

	Demo1 demo;

	while (running) {

		SDL_RenderClear(SDL::Context::renderer);

		const Uint8* kbstate = SDL_GetKeyboardState(NULL);
		if (kbstate[SDL_SCANCODE_Q]) {
			running = false;
		}

		if (kbstate[SDL_SCANCODE_L]) {
			Camera::camera_x += 32;
		}

		if (kbstate[SDL_SCANCODE_K]) {
			Camera::camera_y -= 32;
		}

		if (kbstate[SDL_SCANCODE_J]) {
			Camera::camera_x -= 32;
		}

		if (kbstate[SDL_SCANCODE_I]) {
			Camera::camera_y += 32;
		}

		if (kbstate[SDL_SCANCODE_T]) {
			Camera::camera_angle += 5.0;
		}
		if (kbstate[SDL_SCANCODE_R]) {
			Camera::camera_angle -= 5.0;
		}
		

		while (SDL_PollEvent(&SDL::Context::event)) {
			if (SDL::Context::event.type == SDL_QUIT) {
				running = false;
			}
			if (SDL::Context::event.type == SDL_MOUSEMOTION) {

			}

			if (SDL::Context::event.type == SDL_KEYDOWN) {
				if (SDL::Context::event.key.keysym.sym == SDLK_RIGHT) {
					//soundManager.play("wilhelm");
				}
			}
			if (SDL::Context::event.type == SDL_KEYUP) {
				switch (SDL::Context::event.key.keysym.sym) {
				case SDLK_UP:
				case SDLK_DOWN:
					//pressed = false;
					break;
				}
			}
		}

		demo.handle_input();
		demo.update();
		demo.draw();

		SDL_SetRenderDrawColor(SDL::Context::renderer, 25, 25, 25, 255);
		SDL_RenderPresent(SDL::Context::renderer);


		int delay = (int)fps - (SDL_GetTicks() - startFTime);
		
		if (delay > 0) SDL_Delay(delay);

		startFTime = SDL_GetTicks();
	}

	return 0;

}