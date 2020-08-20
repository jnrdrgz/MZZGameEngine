#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include <random>

#include <unordered_map>
#include "timer.h"

#define KEYPRESS(X)	(X.type == SDL_KEYDOWN)
#define KEYUNPRESS(X) (X.type == SDL_KEYUP)
#define BUTTONPRESS(X) (X.type == SDL_MOUSEBUTTONDOWN)

#define DEBUG 0

const int screen_w = 640;
const int screen_h = 480;
bool pressed = false;

//helpers
int random_between(int mn, int mx) {
	int n = rand() % (mx - mn) + mn;
	return n;
}

float random_betweenf(float min, float max) {
	float scale = rand() / (float)RAND_MAX; // [0, 1.0] 
	return min + scale * (max - min);      // [min, max]
}

namespace SDL {
	struct Context
	{
	public:
		Context(const char* title) {
			SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
			SDL_SetHint("SDL_HINT_ANDROID_SEPARATE_MOUSE_AND_TOUCH", "0");
			
			window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, SDL_WINDOW_SHOWN);
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

			TTF_Init();
			IMG_Init(IMG_INIT_PNG);
			Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 );
			
			font = TTF_OpenFont("umeboshi.ttf", 25);
			if (!font) {
				std::cout << "error loading font\n";
			}
		}

		 ~Context() {
			SDL_DestroyWindow(window);
			SDL_DestroyRenderer(renderer);
			Mix_Quit();
			IMG_Quit();
			TTF_Quit();
			SDL_Quit();
		}

		static SDL_Renderer* renderer;
		static TTF_Font* font;
		static SDL_Event event;
		bool running = false;

	private:
		SDL_Window* window = NULL;
	};
}

struct SoundManager
{
	SoundManager() {
		//chuncks.reserve(10);

		auto load_audio = [](std::string file_path){
			Mix_Chunk* m = Mix_LoadWAV(file_path.c_str());
			if (!m) std::cout << "erro loading audio\n";
			return m;
		};

		chunks["wilhelm"] = load_audio("wilhelm.wav");
		//chuncks.push_back(m);
	}

	~SoundManager() {
		//for (auto c : chuncks) {
		//	Mix_FreeChunk(c);
		//}
	}

	void play(std::string sound) {
		Mix_PlayChannel(-1, chunks[sound], 0);
	}

	//std::vector<> chuncks;

	std::unordered_map<std::string, Mix_Chunk*> chunks;
};

struct Text
{
	Text(std::string text, int x, int y, int w, int h) : text{ text }, rct{ x,y,w,h } {
		load_text();
	}

	Text(std::string text, int x, int y, int h) : text{ text } {
		load_text_and_rct(x,y,h);
	}

	~Text() {
		std::cout << "Destrcutr called\n";
		SDL_DestroyTexture(texture);
	}

	void load_text() {
		SDL_Surface* tmp_sur = TTF_RenderUTF8_Solid(SDL::Context::font, text.c_str(), { 0,0,0,0 });
		texture = SDL_CreateTextureFromSurface(SDL::Context::renderer, tmp_sur);
		SDL_FreeSurface(tmp_sur);
	}

	void load_text_and_rct(int x, int y,int h) {
		SDL_Surface* tmp_sur = TTF_RenderUTF8_Solid(SDL::Context::font, text.c_str(), { 0,0,0,0 });
		texture = SDL_CreateTextureFromSurface(SDL::Context::renderer, tmp_sur);

		rct.x = x;
		rct.y = y;
		rct.w = tmp_sur->w;
		rct.h = h;

		SDL_FreeSurface(tmp_sur);
	}

	void update(std::string text) {
		SDL_DestroyTexture(texture);
		this->text = text;
		load_text();
	}

	void render() {
		SDL_RenderCopy(SDL::Context::renderer, texture, nullptr, &rct);
	}

	std::string text;
	SDL_Texture* texture;
	SDL_Rect rct;
};

namespace Assets {
	struct Sheet
	{
		Sheet() {
			SDL_Surface* tmp_srf = IMG_Load("sheet.png");
			if (!tmp_srf) std::cout << "error creating srf\n";
			texture = SDL_CreateTextureFromSurface(SDL::Context::renderer, tmp_srf);
			if (!texture) std::cout << "error creating texture\n";
		}

		static SDL_Texture* texture;
	};
}


SDL_Renderer* SDL::Context::renderer = NULL;
SDL_Event SDL::Context::event;
SDL_Texture* Assets::Sheet::texture = NULL;
TTF_Font* SDL::Context::font = NULL;

struct GameObject;

struct InputHandler
{
	virtual void handle_input(GameObject& gameobject) = 0;
};

//22 hor
struct GameObject
{
	GameObject(int x, int y, int w, int h, int tilex, int tiley) :
		tile{ tile }, dst{ x,y,w,h }, src{ tilex * 16,tiley * 16,16,16 },
		angle{ 0.0 }, flip{ SDL_FLIP_NONE }{
	}

	void draw() {
		SDL_RenderCopyEx(SDL::Context::renderer, Assets::Sheet::texture, &src, &dst, angle, NULL, flip);
	}
	
	void set_input_handler(std::unique_ptr<InputHandler> ptr) {
		//input_handler.swap(ptr);

		input_handler = std::move(ptr);
	}

	int tile;
	double angle;
	SDL_RendererFlip flip;
	SDL_Rect dst, src;

	std::unique_ptr<InputHandler> input_handler{nullptr};
};

struct State 
{
	virtual void update() = 0;
	virtual void handle_input() = 0;
	virtual void draw() = 0;
};

struct Button
{
	Button(int x, int y, int w, int h, std::string text, int margin) :
		rct{x,y,w,h}, text{text,x+margin,y+margin,w-margin*2,h-margin*2},
		text_str{text},
		selected{false}
	{
		std::cout << "buttoin created: " <<  x << "-" << y << "-" << w << "-" << h << "-" << margin << "\n";
	}

	void draw() {
		SDL_SetRenderDrawColor(SDL::Context::renderer, 0,0,0,0);
		SDL_RenderDrawRect(SDL::Context::renderer, &rct);
		SDL_SetRenderDrawColor(SDL::Context::renderer, 255, 255, 255, 255);
		text.render();
	}

	bool handle_input() {
		if (SDL_GetMouseState(&mx, &my)  & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			if (mx > rct.x && mx<rct.x + rct.w
				&& my > rct.y && my < rct.y + rct.h) {
				return true;
			}
		}

		return false;
	}

	SDL_Rect rct;
	std::string text_str;
	char tag;
	Text text;
	bool selected;
	int mx{0}, my{ 0 };
};

struct Menu
{
	Menu(int x, int y, int w, int h, int padding) {
		buttons.reserve(3);

		int bh = (h / 3);
		buttons.emplace_back(x, y, w, bh, "PLAY", padding);
		buttons.emplace_back(x, y+bh+padding, w, bh, "OPTIONS", padding);
		buttons.emplace_back(x, y+(bh*2)+padding, w, bh, "EXIT", padding);
	}

	int handle_input() {
		for (auto& b : buttons) {
			if (b.handle_input()) {
				if (b.text_str == "PLAY") {
					return 1;
				}
				if (b.text_str == "OPTIONS") {
					return 2;
				}
				if (b.text_str == "EXIT") {
					return 3;
				}
			}
		}

		return 0;
	}

	void draw() {
		for (auto& b : buttons) {
			b.draw();
		}
	}

	std::vector<Button> buttons;
};

struct MenuState : State
{
	MenuState() : menu{ screen_w / 2, screen_h / 2, 100, 45 * 3, 7 }
	{
	}
	
	void update() override{
	}
	
	void handle_input() override{
	
	}
	
	void draw() override{
	
	}


	Menu menu;
};

struct PlayerInputHandler : InputHandler
{
	void handle_input(GameObject& gameobject) {
		const Uint8* kbstate = SDL_GetKeyboardState(NULL);
		if (kbstate[SDL_SCANCODE_RIGHT]) {
			gameobject.dst.x += 5;
		}
		if (kbstate[SDL_SCANCODE_LEFT]) {
			gameobject.dst.x -= 5;
		}
	}
};

struct PlayingState : State
{
	PlayingState() : player{ screen_w / 2,screen_h / 2,32,32,29,0 } {

		player.set_input_handler(std::make_unique<PlayerInputHandler>());
	}
	void update() override {}
	void handle_input() override {
		player.input_handler->handle_input(player);
	}
	void draw() override {
		player.draw();
	}

	GameObject player;

	//physics physics

};

struct Game
{
	Game(){
		
		
	}

	bool update() {
		return true;
	}
	
	bool handle_input() {
		return true;
	}

	void draw() {
	}

	std::unique_ptr<State> state{nullptr};

};

int main(int argc, char* args[])
{
	(void)argc;
	(void)args;

	SDL::Context sdl_context("game");
	Assets::Sheet sheet;
	bool running = true;

	Uint32 startFTime = SDL_GetTicks();
	float fps = (1.0f / 55.0f)*1000.0f;

	std::vector<GameObject> objects;
	std::vector<Text> texts;

	texts.reserve(50);
	
	SoundManager soundManager;

	bool clicked = false;

	Game game;

	Button button1(0, 0, 100, 45, "PLAY", 7);
	Button button2(0, 50, 100, 45, "OPTIONS", 7);
	Button button3(0, 100, 100, 45, "EXIT", 7);

	Menu menu(screen_w/2, screen_h / 2, 100, 45*3, 7);

	while (running) {

		SDL_RenderClear(SDL::Context::renderer);

		const Uint8* kbstate = SDL_GetKeyboardState(NULL);
		if (kbstate[SDL_SCANCODE_Q]) {
			running = false;
		}

		if (kbstate[SDL_SCANCODE_RIGHT]) {
		}

		if (kbstate[SDL_SCANCODE_D] && !pressed) {
			pressed = true;
		}

		while (SDL_PollEvent(&SDL::Context::event)) {
			
			if (KEYUNPRESS(SDL::Context::event)) pressed = false;

			if (SDL::Context::event.type == SDL_QUIT) {
				running = false;
			}
			if (SDL::Context::event.type == SDL_MOUSEMOTION) {

			}
			if (SDL::Context::event.type == SDL_MOUSEBUTTONDOWN) {
				if (!clicked) {
					objects.emplace_back(rand() % screen_w, rand() % screen_h, 32, 32, rand() % 47, rand() % 21);
					if (texts.size() < 20) {
						texts.emplace_back("Desturoy", rand() % screen_w, rand() % screen_h, 25);
					}
					else {
						std::cout << "limit reached\n";
					}
					clicked = true;
				}
			}
			if (SDL::Context::event.type == SDL_MOUSEBUTTONUP) {
				if (clicked) {
					clicked = false;
				}
			}
			if (SDL::Context::event.type == SDL_KEYDOWN) {
				if (SDL::Context::event.key.keysym.sym == SDLK_RIGHT) {
					soundManager.play("wilhelm");
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

		menu.handle_input();
		game.handle_input();
		game.draw();
		menu.draw();


		button1.draw();
		button2.draw();
		button3.draw();

		for (auto& g : objects) {
			g.draw();
		}

		for (auto& t : texts) {
			t.render();
		}
		
		SDL_SetRenderDrawColor(SDL::Context::renderer, 125, 125, 0, 255);
		SDL_RenderPresent(SDL::Context::renderer);


		int delay = (int)fps - (SDL_GetTicks() - startFTime);
		
		if (delay > 0) SDL_Delay(delay);

		startFTime = SDL_GetTicks();
	}

	return 0;

}