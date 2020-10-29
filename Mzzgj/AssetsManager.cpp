#include "AssetsManager.h"

SDL_Texture* Assets::Sheet::texture = NULL;

Assets::Sheet::Sheet() {
	SDL_Surface* tmp_srf = IMG_Load("sheet.png");
	if (!tmp_srf) std::cout << "error creating srf\n";
	texture = SDL_CreateTextureFromSurface(SDL::Context::renderer, tmp_srf);
	if (!texture) std::cout << "error creating texture\n";
}