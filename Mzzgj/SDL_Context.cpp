#include "SDL_Context.h"

SDL_Renderer* SDL::Context::renderer = NULL;
SDL_Event SDL::Context::event;
TTF_Font* SDL::Context::font = NULL;
int SDL::Context::screen_w = 0;
int SDL::Context::screen_h = 0;

SDL::Context::Context(const char* title, int screen_w, int screen_h) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	SDL_SetHint("SDL_HINT_ANDROID_SEPARATE_MOUSE_AND_TOUCH", "0");

	this->screen_w = screen_w;
	this->screen_h = screen_h;

	window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

	TTF_Init();
	IMG_Init(IMG_INIT_PNG);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	font = TTF_OpenFont("roboto.ttf", 25);
	if (!font) {
		std::cout << "error loading font\n";
	}
}

SDL::Context::~Context() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	Mix_Quit();
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}