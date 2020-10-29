#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <unordered_map>

#include "timer.h"
#include "vector2.h"

const int screen_w = 640;
const int screen_h = 480;
bool pressed = false;

//camera-- moves to right why dunno
int camera_x = 0;
int camera_y = 0;
double camera_angle = 0.0;

#define DEBUG 0

void LOG(std::string where_, std::string message) {
#if DEBUG
	std::cout << "[" << where_ << "]" << " " << message << "\n";
#endif // DEBUG

}

//helpers
int random_between(int mn, int mx) {
	if (mn == mx) return mn;
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


///////////////
//particles////
///////////////

struct Text
{
	Text(std::string text, int x, int y, int w, int h) : text{ text }, rct{ x,y,w,h }, color{ 255,255,255,255 } {
		load_text();
	}

	Text(std::string text, int x, int y, int w, int h, SDL_Color color) : text{ text }, rct{ x,y,w,h }, color{ color } {
		load_text();
	}

	Text(std::string text, int x, int y, int h) : text{ text }, color{ 255,255,255,255 }{
		load_text_and_rct(x,y,h);
	}

	Text(std::string text, int x, int y, int h, SDL_Color color) : text{ text }, color{ color }{
		load_text_and_rct(x, y, h);
	}

	~Text() {
		std::cout << "Destrcutr called\n";
		SDL_DestroyTexture(texture);
	}

	void load_text() {
		SDL_Surface* tmp_sur = TTF_RenderUTF8_Solid(SDL::Context::font, text.c_str(), color);
		texture = SDL_CreateTextureFromSurface(SDL::Context::renderer, tmp_sur);
		SDL_FreeSurface(tmp_sur);
	}

	void load_text_and_rct(int x, int y,int h) {
		SDL_Surface* tmp_sur = TTF_RenderUTF8_Solid(SDL::Context::font, text.c_str(), color);
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

	SDL_Color color;
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



struct Particle
{
	Particle(int x, int y, int w, int h, int tx, int ty, Uint32 lifetime,
		float gx, float gy) :
		dst{ x,y,w,h },
		src{ 16 * tx, 16 * ty, 16,16 },
		dst_camera{ x + camera_x,y + camera_y,w,h },
		tx{ tx }, ty{ ty },
		gx{ gx }, gy{ gy }, active{ true }, lifetime{ lifetime }
	{
		angle = 0.0f;
		life.start();
	}

	void update() {
		ax += gx;
		ay += gy;
		vx += ax;
		vy += ay;

		dst.x += (int)vx;
		dst.y += (int)vy;

		if (life.getTicks() >= lifetime) {
			//std::cout << "life ended\n";
			active = false;
		}
	}

	//here_goes
	void draw() {
		if (active) {
			dst_camera.x = dst.x + camera_x;
			dst_camera.y = dst.y + camera_y;
			SDL_RenderCopyEx(SDL::Context::renderer, Assets::Sheet::texture, &src, &dst_camera, angle, NULL, SDL_FLIP_NONE);

		}
	}

	Uint32 lifetime;
	Timer life;
	SDL_Rect src, dst, dst_camera;
	float gx, gy, ax{ 0.0f }, ay{ 0.0f }, vx{ 0.0f }, vy{ 0.0f }, angle{ 0.0f };
	bool gdown, active;
	int tx, ty;
};

//particles
struct ParticleEmitter
{
	ParticleEmitter() : MAX_PARTICLES{ 200 } {
		particles.reserve(MAX_PARTICLES);
	}

	ParticleEmitter(int range_x, int range_y,
		int min_w, int max_w, int min_h, int max_h,
		int tx, int ty, float gx, float gy, Uint32 lifetime)
		: range_x{ range_x }, range_y{ range_y },
		min_w{ min_w }, max_w{ max_w }, min_h{ min_h }, max_h{ max_h },
		tx{ tx }, ty{ ty }, gx{ gx }, gy{ gy }, lifetime{ lifetime }, finished{ false },
		MAX_PARTICLES{ 200 } {


		particles.reserve(MAX_PARTICLES);
		emitting = false;
	}

	ParticleEmitter(int range_x, int range_y,
		int max_size, int min_size,
		int tx, int ty, float gx, float gy, Uint32 lifetime)
		: range_x{ range_x }, range_y{ range_y },
		min_w{ min_size }, max_w{ max_size }, min_h{ min_size }, max_h{ max_size },
		tx{ tx }, ty{ ty }, gx{ gx }, gy{ gy }, lifetime{ lifetime }, finished{false},
		MAX_PARTICLES{ 200 } {


		particles.reserve(MAX_PARTICLES);
		emitting = false;
	}


	void emit(int x, int y, Uint32 emitter_lifetime) {
		//if (particles.size() < MAX_PARTICLES) {
		//	particles.emplace_back(
		//		random_between(x - range_x, x + range_x),
		//		random_between(y - range_y, y + range_y),
		//		random_between(min_w, max_w),
		//		random_between(min_h, max_h),
		//		15, 10,
		//		lifetime, gx, gy);
		//}
		particles.clear();
		this->emitter_lifetime = emitter_lifetime;
		life.start();

		this->x = x;
		this->y = y;
		//std::cout << "start emitting: " << x << " - " << y << "\n";
		emitting = true;
	}

	virtual void push_particle() = 0;

	virtual void update() = 0;

	void draw() {
		for (auto& p : particles) {
			p.draw();
		}
	}

	Uint32 lifetime;
	Timer life;
	Uint32 emitter_lifetime{0};
	std::vector<Particle> particles;
	const size_t MAX_PARTICLES;

	int x, y, range_x, range_y, min_w, max_w, min_h, max_h, tx, ty;
	float gx, gy;
	bool emitting;
	bool finished;
};

struct EnemyParticleEmitter : ParticleEmitter {

	EnemyParticleEmitter() : ParticleEmitter(10, 10, 10, 10, 10, 15, 0.0f, -0.05f, 500)
	{
	}

	void push_particle() override {
		if (particles.size() < MAX_PARTICLES) {
			particles.emplace_back(
				random_between(x - range_x, x + range_x),
				random_between(y - range_y, y + range_y),
				random_between(min_w, max_w),
				random_between(min_h, max_h),
				15, 10,
				lifetime, gx, gy);
		}
	}

	void update() override {
		if (emitting) {
			push_particle();
			if (life.getTicks() > emitter_lifetime) {
				emitting = false;
				finished = true;
				life.stop();
			}
		}


		for (auto& p : particles) {
			p.update();
		}
		std::erase_if(particles, [](Particle& p) { return !p.active; });
	}
};

//TODO
struct EnemyCrossParticleEmitter : ParticleEmitter {

	EnemyCrossParticleEmitter() : ParticleEmitter(4, 4, 10, 10, 10, 15, 0.0f, -0.05f, 500)
	{
		
	}

	void push_particle() override {
		if (particles.size() < MAX_PARTICLES) {
			float _gy = random_betweenf(-0.1f, 0.3f );
			float _gx = random_betweenf(-0.1f, 0.1f);
			random_between(0, 2) == 0 ? _gx = 0.0f : _gy = 0.0f;
			particles.emplace_back(
				random_between(x - range_x, x + range_x),
				random_between(y - range_y, y + range_y),
				random_between(min_w, max_w),
				random_between(min_h, max_h),
				15, 10,
				lifetime, _gx, _gy);
		}
	}

	void update() override {
		if (emitting) {
			push_particle();
			if (life.getTicks() > emitter_lifetime) {
				emitting = false;
				finished = true;
				life.stop();
			}
		}


		for (auto& p : particles) {
			p.update();
		}

		std::erase_if(particles, [](Particle& p) { return !p.active; });
	}
};

struct GameObject;

struct InputHandler
{
	virtual void handle_input(GameObject& gameobject) = 0;

	bool pressed[512]{false};
	SDL_Scancode last_scancode;
};

struct AI
{
	virtual void update(GameObject& gameobject) = 0;
};

struct Animation
{
	virtual void update(GameObject& gameobject) = 0;

	Timer timer;
};

struct Emitter
{
	Emitter(GameObject& gameobject) : gameobject{gameobject} {
		
	}

	virtual void emit() = 0;
	virtual void update() = 0;
	virtual void draw() = 0;

	//std::unique_ptr<ParticleEmitter> emitter{ nullptr };
	ParticleEmitter* emitter{ nullptr };
	GameObject& gameobject;
};

//22 hor
struct GameObject
{
	GameObject(int x, int y, int w, int h, int tilex, int tiley, std::string tag) :
		tile{ tile }, dst{ x,y,w,h }, dst_camera{x,y,w,h}, src{ tilex * 16,tiley * 16,16,16 },
		angle{ 0.0 }, flip{ SDL_FLIP_NONE }, tag{ tag }, active{true}{
	}

	void draw() {
		if (active) {
			dst_camera.x = dst.x + camera_x;
			dst_camera.y = dst.y + camera_y;
			SDL_RenderCopyEx(SDL::Context::renderer, Assets::Sheet::texture, &src, &dst_camera, angle+camera_angle, NULL, flip);
		}

		if (emitter) emitter->draw();
	}
	
	void set_input_handler(std::unique_ptr<InputHandler> ptr) {
		input_handler = std::move(ptr);
	}

	void set_ai(std::unique_ptr<AI> ptr) {
		ai = std::move(ptr);
	}

	void set_animation(std::unique_ptr<Animation> ptr) {
		animation = std::move(ptr);
	}

	void set_emitter(std::unique_ptr<ParticleEmitter> ptr) {
		emitter = std::move(ptr);
	}

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

struct Button
{
	Button(int x, int y, int w, int h, std::string text, int margin) :
		rct{x,y,w,h}, text{text,x+margin,y+margin,w-margin*2,h-margin*2},
		text_str{text},
		selected{false},
		color{255,0,0,255}
	{
		std::cout << "buttoin created: " <<  x << "-" << y << "-" << w << "-" << h << "-" << margin << "\n";
		
	}

	void draw() {
		SDL_SetRenderDrawColor(SDL::Context::renderer, color.r,color.g,color.b,color.a);
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
			if(SDL_GetMouseState(&mx, &my) && SDL_BUTTON(SDL_BUTTON_LEFT)) return true;
		} else {
			color.r = 255;
		}
		

		return false;
	}

	SDL_Rect rct;
	std::string text_str;
	char tag;
	Text text;
	bool selected;
	int mx{0}, my{ 0 };
	SDL_Color color;
};

struct Menu
{
	Menu(int x, int y, int w, int h, int padding) {
		buttons.reserve(3);

		int bh = (h / 3);		
		int by = y;
		buttons.emplace_back(x, by, w, bh, "PLAY", padding);
		by += bh+padding;
		buttons.emplace_back(x, by, w, bh, "OPTIONS", padding);
		by += bh+padding;
		buttons.emplace_back(x, by, w, bh, "EXIT", padding);
	}

	int handle_input() {
		for (auto& b : buttons) {
			if (b.handle_input()) {
				if (b.text_str == "PLAY") {
					SDL_Log("PLAY");
					return 1;
				}
				if (b.text_str == "OPTIONS") {
					SDL_Log("OPTIONS");
					return 2;
				}
				if (b.text_str == "EXIT") {
					SDL_Log("EXIT");
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

///////////////////
//////STATES///////
///////////////////
struct State
{
	virtual std::unique_ptr<State> update() = 0;
	virtual std::unique_ptr<State> handle_input() = 0;
	virtual void draw() = 0;
};

struct PlayingState;
struct MenuState : State
{
	MenuState() : menu{ (screen_w / 2)-100, (screen_h / 2)-45*3, 100, 45 * 3, 7 }
	{
	}
	
	std::unique_ptr<State> update() override{
		return nullptr;
	}
	
	std::unique_ptr<State> handle_input() override;
	
	void draw() override{
		menu.draw();
	}

	Menu menu;
};

struct EnemyDeathEmitter : Emitter
{
	EnemyDeathEmitter(GameObject& gameobject) 
		: Emitter(gameobject) {
		//auto e = std::make_unique<ParticleEmitter>( 10, 10, 10, 10, 10, 15, 0.0f, -0.05f, 500);
		//emitter = std::move(e);

		//emitter = new ParticleEmitter(10, 10, 10, 10, 10, 15, 0.0f, -0.05f, 500);
	}

	void emit() override {
		emitter->emit(gameobject.dst.x, gameobject.dst.y, 2000);
	}

	void update() override{
		emitter->update();
	}
	void draw() override {
		emitter->draw();
	}
};

struct PlayerInputHandler : InputHandler
{
	void hop(GameObject& gameobject) {
		if (!hoping) {
			//gameobject.dst.y += 2;
			//gameobject.angle += 12;
			hoping = true;
			//gameobject.angle += _hoping;
			//_hoping * -1;
		}
		else {
			//gameobject.dst.y -= 2;
			//gameobject.angle -= 12;
			hoping = false;
		}

		gameobject.angle += _hoping;
		_hoping *= -1;

	}

	void handle_input(GameObject& gameobject) {
		const Uint8* kbstate = SDL_GetKeyboardState(NULL);

		//PLAYER MOVEMENT	
		if (kbstate[SDL_SCANCODE_RIGHT] && !pressed[SDL_SCANCODE_RIGHT]) {
			gameobject.dst.x += 32;
			if (gameobject.dst_camera.x > screen_w-64) {
				camera_x -= 32;
			}


			hop(gameobject);
			gameobject.flip = SDL_FLIP_NONE;
			last_scancode = SDL_SCANCODE_RIGHT;
			pressed[SDL_SCANCODE_RIGHT] = true;
		}
		if (!kbstate[SDL_SCANCODE_RIGHT]) {
			pressed[SDL_SCANCODE_RIGHT] = false;
		}

		if (kbstate[SDL_SCANCODE_LEFT] && !pressed[SDL_SCANCODE_LEFT]) {
			gameobject.dst.x -= 32;
			if (gameobject.dst_camera.x < 64) {
				camera_x += 32;
			}
			hop(gameobject);
			gameobject.flip = SDL_FLIP_HORIZONTAL;
			last_scancode = SDL_SCANCODE_LEFT;
			pressed[SDL_SCANCODE_LEFT] = true;
		}
		if (!kbstate[SDL_SCANCODE_LEFT]) {
			pressed[SDL_SCANCODE_LEFT] = false;
		}

		if (kbstate[SDL_SCANCODE_UP] && !pressed[SDL_SCANCODE_UP]) {
			gameobject.dst.y -= 32;
			if (gameobject.dst_camera.y < 64) {
				camera_y += 32;
			}
			//camera_y += 32;
			last_scancode = SDL_SCANCODE_UP;
			hop(gameobject);


			pressed[SDL_SCANCODE_UP] = true;
		}
		if (!kbstate[SDL_SCANCODE_UP]) {
			pressed[SDL_SCANCODE_UP] = false;
		}

		if (kbstate[SDL_SCANCODE_DOWN] && !pressed[SDL_SCANCODE_DOWN]) {
			gameobject.dst.y += 32;

			if (gameobject.dst_camera.y > screen_h - 64) {
				camera_y -= 32;
			}

			hop(gameobject);
			last_scancode = SDL_SCANCODE_DOWN;
			pressed[SDL_SCANCODE_DOWN] = true;
		}
		if (!kbstate[SDL_SCANCODE_DOWN]) {
			pressed[SDL_SCANCODE_DOWN] = false;
		}

		if (kbstate[SDL_SCANCODE_S] && !pressed[SDL_SCANCODE_S]) {
			SoundManager::play_chunk("wilhelm");
			pressed[SDL_SCANCODE_S] = true;
		}
		if (!kbstate[SDL_SCANCODE_S]) {
			pressed[SDL_SCANCODE_S] = false;
		}

		if (kbstate[SDL_SCANCODE_Z] && !pressed[SDL_SCANCODE_Z]) {
			SoundManager::play_chunk("sword");
			pressed[SDL_SCANCODE_Z] = true;
			shooting = true;
		}
		if (!kbstate[SDL_SCANCODE_Z]) {
			pressed[SDL_SCANCODE_Z] = false;
		}
	}

	bool hoping = false;
	float _hoping = -15.0f;
	bool shooting = false;
};


// ////////////////////////////////
////////////////////AIs////////////
// ////////////////////////////////

struct EnemyChaseTargetAI : AI {
	EnemyChaseTargetAI(GameObject& target) : 
		target{target},
		v{ 4 }, vx{ v }, vy{ v } {}

	void update(GameObject& gameobject) override {
		if (target.dst.x > gameobject.dst.x) {
			vx = v;
		}

		if (target.dst.x < gameobject.dst.x) {
			vx = -v;
		}

		if (target.dst.y > gameobject.dst.y) {
			vy = v;
		}


		if (target.dst.y < gameobject.dst.y) {
			vy = -v;
		}

		gameobject.dst.x += vx;
		gameobject.dst.y += vy;
	}

	GameObject& target;
	int vx, vy, v;
};

struct EnemyChaseTargetTimedAI : AI {
	EnemyChaseTargetTimedAI(GameObject& target) :
		target{ target },
		v{ 32 }, vx{ v }, vy{ v }, walk_interval {1200}{
		
		walking_timer.start();
	}

	void update(GameObject& gameobject) override {
		if (target.dst.x > gameobject.dst.x) {
			vx = v;
		}

		if (target.dst.x < gameobject.dst.x) {
			vx = -v;
		}

		if (target.dst.y > gameobject.dst.y) {
			vy = v;
		}


		if (target.dst.y < gameobject.dst.y) {
			vy = -v;
		}

		if (walking_timer.getTicks() >= walk_interval) {
			gameobject.dst.x += vx;
			gameobject.dst.y += vy;
			walking_timer.stop();
			walking_timer.start();
		}
	}

	GameObject& target;
	int vx, vy, v;
	Timer walking_timer;
	Uint32 walk_interval;
};


struct EnemyGoToTheGoldAI : AI {
	EnemyGoToTheGoldAI(GameObject& target) :
		target{ target },
		v{ 4 }, vx{ v }, vy{ 0 } {}

	void update(GameObject& gameobject) override {
		if (target.dst.x > gameobject.dst.x) {
			vx = v;
		}

		if (target.dst.x < gameobject.dst.x) {
			vx = -v;
		}

		gameobject.dst.x += vx;
		gameobject.dst.y += vy;
	}

	GameObject& target;
	int vx, vy, v;
	Timer walking_timer;
	Uint32 walk_interval;
};

struct EnemyMoveRandomlyAI : AI {
	EnemyMoveRandomlyAI() : v{ 4 } , vx{ v }, vy{ v } {}

	void update(GameObject& gameobject) override {
		gameobject.dst.x += vx;
		gameobject.dst.y += vy;

		if (gameobject.dst.y > screen_h) {
			vy = -v;
		}
		if (gameobject.dst.y < 0) {
			vy = v;
		}
		if (gameobject.dst.x > screen_w) {
			vx = -v;
		}
		if (gameobject.dst.y < 0) {
			vx = v;
		}
	}

	int v, vx, vy;
};

struct EnemyMoveRandomlyTimedAI : AI {
	EnemyMoveRandomlyTimedAI() : v{ 32 }, vx{ v }, vy{ v }, walk_interval{ 700 }{

		walking_timer.start();
	}

	void update(GameObject& gameobject) override {
		
		if (gameobject.dst.y > screen_h) {
			vy = -v;
		}
		if (gameobject.dst.y < 0) {
			vy = v;
		}
		if (gameobject.dst.x > screen_w) {
			vx = -v;
		}
		if (gameobject.dst.y < 0) {
			vx = v;
		}
		if (walking_timer.getTicks() >= walk_interval) {
			gameobject.dst.x += vx;
			gameobject.dst.y += vy;
			if (random_between(0, 7) == 0) vx = -v;
			if (random_between(0, 7) == 0) vy = -v;

			walking_timer.stop();
			walking_timer.start();
		}
	}

	int v, vx, vy;
	Timer walking_timer;
	Uint32 walk_interval;
};

struct NormalBulletAI : AI {
	NormalBulletAI(GameObject& player) :
		v{10},
		player{ player }
	{
		orientation = player.flip;
	}

	void update(GameObject& gameobject) override {
		/*
		
		Acá para hacer esto
		if(player.input_handler->shooting)

		me falta una capaz mas de abstraccion
		porque las struct input_handler deberia ser una 
		interfaz mas generica, de ahi tendria que heredar una
		player_input_handler que tendria la propiedad shooting
		y a su vez de gameobject tendria que heredar un player
		que tenga como propiedad una player_input_handler (creo)

		*/

		if(orientation == SDL_FLIP_NONE){
			gameobject.dst.x += v;
			gameobject.angle += 3;
		}
		else {
			gameobject.dst.x -= v;
			gameobject.angle -= 3;
		}
	}

	int v;
	GameObject& player;
	SDL_RendererFlip orientation;
};

struct BoomerangBulletAI : AI{
	BoomerangBulletAI(GameObject & player) :
		v{10},
		player{ player }
	{
		orientation = player.flip;
	}

	void update(GameObject & gameobject) override {
		if (orientation == SDL_FLIP_NONE) {
			gameobject.dst.x += v;
			gameobject.angle += 3;
		}
		else {
			gameobject.dst.x -= v;
			gameobject.angle -= 3;
		}
	}

	int v, starting_point;
	GameObject& player;
	SDL_RendererFlip orientation;
};

struct EnemySpawnerAI : AI {
	EnemySpawnerAI(std::vector<GameObject>& enemy_vec, Uint32 respawn_interval, int min, int max) :
		enemy_vec{ enemy_vec }, respawn_interval{ respawn_interval }, min{ min }, max{max}
	{
		respawn_timer.start();
	}

	void add_enemy(int x, int y) {
		auto& e = enemy_vec.emplace_back(x, y, 32, 32, random_between(25, 32), 6, "enemy");
		//randomize ai
		e.set_ai(std::make_unique<EnemyMoveRandomlyTimedAI>());
		e.set_emitter(std::make_unique<EnemyCrossParticleEmitter>());
	}

	void update(GameObject& gameobject) override {
		
		if (respawn_timer.getTicks() > respawn_interval) {
			for (int i = 0; i < random_between(min,max); i++) {
				add_enemy(gameobject.dst.x, gameobject.dst.y);
			}
			respawn_timer.stop();
			respawn_timer.start();
		}
	}

	std::vector<GameObject>& enemy_vec;
	Uint32 respawn_interval;
	Timer respawn_timer;
	int min, max;
};

struct NormalBombUpdateAI : AI {
	NormalBombUpdateAI() {
		timer.start();
	}

	void update(GameObject& gameobject) override {

	}

	Timer timer;
};

struct SideCollision
{
	SideCollision() : left{ false }, right{ false }, up{ false }, down{ false } {}
	SideCollision(bool left, bool right, bool up, bool down)
		: left{ left }, right{ right }, up{ up }, down{down}
	{}
	
	bool left, right, up, down;
};

struct Physics
{
	bool collision(GameObject& a, GameObject& b) {
		if (a.dst.x < b.dst.x + b.dst.w &&
			a.dst.x + a.dst.w > b.dst.x &&
			a.dst.y < b.dst.y + b.dst.h &&
			a.dst.y + a.dst.h > b.dst.y)
		{
			return true;
		}

		return false;
	}


	bool collision(SDL_Rect& a, SDL_Rect& b) {
		if (a.x < b.x + b.w &&
			a.x + a.w > b.x &&
			a.y < b.y + b.h &&
			a.y + a.h > b.y)
		{
			return true;
		}

		return false;
	}

	std::string collision_tag(GameObject& a, GameObject& b) {
		return b.tag;
	}

	//doesnt check because I dont need it for now
	bool check_boundaries(GameObject& a) {
		if (a.dst.x < 0 || a.dst.x + a.dst.w > screen_w ||
			a.dst.x < 0 || a.dst.x + a.dst.w > screen_w) {
			return true;
		}

		return false;
	}

	bool check_boundaries(GameObject& a, int max_x) {
		//std::cout << a.dst.x << " - " << a.dst.y << "\n";

		if (a.dst.x < 0 || a.dst.x + a.dst.w > max_x) {
			return true;
		}

		return false;
	}

	GameObject* collision(std::vector<GameObject>& group, GameObject& obj) {
		for (auto& g : group) {
			if (collision(g, obj)) {
				return &g;
			}
		}
		
		return nullptr;
	}

	enum collsion_direction { LEFT, RIGHT, DOWN, UP };
	//collsion_direction foresight(SDL_Rect &a, SDL_Rect& b) {
	//	SDL_Rect ret_rct = {a.x,a.y,a.w,a.h};
	//	
	//	ret_rct.x += 32;
	//
	//	ret_rct.x -= 32;
	//	ret_rct.y += 32;
	//	ret_rct.y -= 32;
	//	
	//}

	SideCollision pre_collision(GameObject& a, GameObject& b) {
		SideCollision side_col{ };
		
		a.dst.x += 32;
		if (collision(a.dst, b.dst)) {
			a.dst.x -= 32;
			side_col.left = true;
		}

		a.dst.x -= 32;
		if (collision(a.dst, b.dst)) {
			a.dst.x += 32;
			side_col.right = true;
		}

		a.dst.y -= 32;
		if (collision(a.dst, b.dst)) {
			a.dst.y += 32;
			side_col.up = true;
		}

		a.dst.y += 32;
		if (collision(a.dst, b.dst)) {
			a.dst.x -= 32;
			side_col.down = true;
		}

		return side_col;
	}

	bool collision(std::vector<GameObject>& group1, std::vector<GameObject>& group2) {
		for (auto& g1 : group1) {
			for (auto& g2 : group2) {
				if (collision(g1, g2)) {
					return true;
				}
			}
		}

		return false;
	}

	void wall_collision(GameObject& go1, std::vector<GameObject>& go2) {
		for (auto& g : go2) {
			if (go1.dst.x == g.dst.x && go1.dst.y == g.dst.y) {
				//auto* ih = go1.input_handler.get();
				switch (go1.input_handler->last_scancode) {
				case SDL_SCANCODE_UP:
					std::cout << "Up\n";
					go1.dst.y += 32;
					break;
				case SDL_SCANCODE_DOWN:
					std::cout << "Down\n";
					go1.dst.y -= 32;
					break;
				case SDL_SCANCODE_LEFT:
					go1.dst.x += 32;
					std::cout << "Left\n";
					break;
				case SDL_SCANCODE_RIGHT:
					go1.dst.x -= 32;
					std::cout << "Right\n";
					break;

				default:
					std::cout << "Nothing\n";
				}
			}
		}
	}
};

struct Level
{
	Level(std::string map) :
		player{ 0,0,32,32,29,0, "player" },
		enemies_killed{ 0 }
	{
		player.set_input_handler(std::make_unique<PlayerInputHandler>());
		
		//camera_x = 32*30;
		//camera_y = 32*30;
		//camera_x = 0;
		//camera_y = 0;
		things_to_protect.reserve(20);
		enemies.reserve(100);
		bullets.reserve(50);
		bombs.reserve(50);
		walls.reserve(100);
		enemy_spawners.reserve(20);

		parse_map(map);

		enemy_spawn_timer.start();
		levelTimer.start();
	}

	void add_enemy_spawner(int x, int y) {
		auto& es = enemy_spawners.emplace_back(x, y, 32, 32, 25, 13, "spawner");
		es.set_ai(std::make_unique<EnemySpawnerAI>(enemies, 1000*15, 3, 10));
	}

	void add_wall(int x, int y) {
		auto& w = walls.emplace_back(x, y, 32, 32, 0, 1, "wall");
	}

	void add_thing_to_protect(int x, int y) {
		things_to_protect.emplace_back(x, y, 32, 32, 41, 4, "gold");
	}

	void add_reference_tile(int x, int y) {
		reference_tiles.emplace_back(x, y, 32, 32, random_between(1, 5),0, "r_tile");
	}

	void parse_map(std::string file_map_path) {
		std::ifstream stream(file_map_path);
		if (!stream) LOG("PARSE MAP", "error map file not found\n");
		std::string line;

		int x = 0;
		int y = 0;
		while (getline(stream, line)) {
			for (char c : line) {
				if (c != '\n') {
					switch (c) {
					case 'S':
						add_enemy_spawner(x, y);
						break;
					case 'G':
						add_thing_to_protect(x, y);
						break;
					case 'P':
						player.dst.x = x;
						camera_x = -(player.dst.x);

						player.dst.y = y;
						camera_y = -(player.dst.y);
						break;

					case '1':
						add_wall(x, y);
						break;
					case '2':
						add_reference_tile(x, y);
						break;
					case '0':
						break;
					default:
						std::cout << "unknown tile\n";
						break;
					}
				}

				x+=32;
			}
			y+=32;
			x=0;
		}
	}

	void base_update() {

		//fix sword, stop apperieng, somthing with the limit i gues
		//or directly make it boomerang
		for (auto& b : bullets) {
			if (b.ai) b.ai->update(b);
			if (physics.check_boundaries(b, screen_w * 3)) {
				b.active = false;
			}
		}

		std::erase_if(bullets, [](GameObject& g) { return !g.active; });

		//GameObject* obj = 
		physics.wall_collision(player, walls);
		//if (obj) {
		//
		//	player.dst.x = obj->dst.x+32;
		//}

	

		for (auto& e : enemies) {
			if (e.active) {
				if (e.ai) e.ai->update(e);
			}

			GameObject* obj = physics.collision(bullets, e);
			if (obj) {
				obj->active = false;
				e.active = false;
				if (!e.active) {
					enemies_killed++;
					std::cout << enemies_killed << "\n";
				}
				if (e.emitter) {
					if (!e.emitter->emitting) {
						e.emitter->emit(e.dst.x, e.dst.y, 2000);
					}
				}
			};

			GameObject* protectable = physics.collision(things_to_protect, e);
			if (protectable) {
				protectable->active = false;
				//e.active = false;
			}
			if (e.emitter) e.emitter->update();
		}

		std::erase_if(enemies, [&](GameObject& g) {
			if (!g.emitter) {
				return !g.active;
			}
			else {
				return !g.active && g.emitter->finished;
			}
		});

		for (auto& es : enemy_spawners) {
			es.ai->update(es);
		}

	}

	virtual std::unique_ptr<State> update() = 0;
	
	void base_handle_input(){
		if (player.input_handler) player.input_handler->handle_input(player);

		//my senses say that this is not okay
		PlayerInputHandler* pih = (PlayerInputHandler*)player.input_handler.get();
		if (pih->shooting) {
			if (bullets.size() < 5) {
				auto& b = bullets.emplace_back(player.dst.x, player.dst.y, 32, 32, 35, 6, "bullet");
				b.set_ai(std::make_unique<NormalBulletAI>(player));
			}
			else {
				std::cout << "max 5 bullets!!\n";
			}
			pih->shooting = false;


		}

		const Uint8* kbstate = SDL_GetKeyboardState(NULL);
		//if (kbstate[SDL_SCANCODE_ESCAPE]) {
		//	return std::make_unique<MenuState>();
		//}

	}
	virtual std::unique_ptr<State> handle_input() = 0;

	void draw() {
		for (auto& r : reference_tiles) {
			r.draw();
		}
		player.draw();
		for (auto& e : enemies) {
			e.draw();
		}
		for (auto& t : things_to_protect) {
			t.draw();
		}
		for (auto& b : bullets) {
			b.draw();
		}
		for (auto& b : bombs) {
			b.draw();
		}
		for (auto& w : walls) {
			w.draw();
		}
		for (auto& es : enemy_spawners) {
			es.draw();
		}
	}

	//boomerang sword?
	GameObject player;
	std::vector<GameObject> enemies;
	std::vector<GameObject> things_to_protect;
	std::vector<GameObject> bullets;
	std::vector<GameObject> bombs;
	std::vector<GameObject> walls;
	std::vector<GameObject> enemy_spawners;
	std::vector<GameObject> reference_tiles; //tiles that show that the player is moving
	Timer enemy_spawn_timer; //this have to go in enemy spawner ai
	Physics physics;
	Timer levelTimer;
	int enemies_killed,coins_left;
};

struct LostState : State
{
	LostState() :
		msg{ "YOU LOST", screen_w / 2, screen_h / 2, 50, {255,125,0,0} }
	{
		timer.start();
		limit = 1000 * 10;
		music_channel = SoundManager::play_for_seconds("hipintro", limit);

		msg.rct.y = 25;
		msg.rct.y = 350;
	}

	std::unique_ptr<State> update() override {
		if (timer.getTicks() >= limit) {
			timer.stop();
			//return std::make_unique<PlayingState>();
		}

		return nullptr;
	}

	std::unique_ptr<State> handle_input() override {
		const Uint8* kbstate = SDL_GetKeyboardState(NULL);
		if (kbstate[SDL_SCANCODE_SPACE]) {
			timer.stop();
			SoundManager::shut_up_channel(music_channel);
			//return std::make_unique<PlayingState>();
		}

		return nullptr;
	}

	void draw() override {
		msg.render();
	}

	Text msg;
	Timer timer;
	int limit, music_channel;
};

struct WonState : State
{
	WonState() :
		msg{ "YOU WON", screen_w / 2, screen_h / 2, 50, {255,125,0,0} }
	{
		timer.start();
		limit = 1000 * 10;
		music_channel = SoundManager::play_for_seconds("hipintro", limit);

		msg.rct.y = 25;
		msg.rct.y = 350;
	}

	std::unique_ptr<State> update() override {
		if (timer.getTicks() >= limit) {
			timer.stop();
			//return std::make_unique<PlayingState>();
		}

		return nullptr;
	}

	std::unique_ptr<State> handle_input() override {
		const Uint8* kbstate = SDL_GetKeyboardState(NULL);
		if (kbstate[SDL_SCANCODE_SPACE]) {
			timer.stop();
			SoundManager::shut_up_channel(music_channel);
			//return std::make_unique<PlayingState>();
		}

		return nullptr;
	}

	void draw() override {
		msg.render();
	}

	Text msg;
	Timer timer;
	int limit, music_channel;
};


struct LevelOne : Level
{
	LevelOne() : Level{"level1"} {
		
	}

	std::unique_ptr<State> update() override{
		base_update();

		if (enemies_killed >= 10) {
			return std::make_unique<WonState>();
		}

		return nullptr;
	}

	std::unique_ptr<State> handle_input() override {
		base_handle_input();

		return nullptr;
	}
};

struct PlayingState : State
{
	PlayingState() : level{ std::make_unique<LevelOne>() } {
		
	}

	std::unique_ptr<State> update() override {
		if(level) return level->update();
		//return nullptr;
	}

	std::unique_ptr<State> handle_input() override {
		if(level) return level->handle_input();
		//return nullptr;
	}

	void draw() override {
		if(level) level->draw();
	}

	std::unique_ptr<Level> level;
};

struct IntroState : State
{
	IntroState() : 
		gameName{ "A GAME", screen_w / 2, screen_h / 2, 50, {255,125,0,0} },
		levelName{ "LEVEL 1: PROTECT THE GOLD", screen_w / 2, screen_h / 2, 50, {255,125,0,0} }
	{
		timer.start();
		limit = 1000 * 10;
		music_channel = SoundManager::play_for_seconds("hipintro", limit);

		gameName.rct.x = 25;
		gameName.rct.y = 25;

		levelName.rct.x = 25;
		levelName.rct.y = 350;
	}

	std::unique_ptr<State> update() override {
		if (timer.getTicks() >= limit) {
			timer.stop();
			return std::make_unique<PlayingState>();
		}

		return nullptr;
	}

	std::unique_ptr<State> handle_input() override {
		const Uint8* kbstate = SDL_GetKeyboardState(NULL);
		if (kbstate[SDL_SCANCODE_SPACE]) {
			timer.stop();
			SoundManager::shut_up_channel(music_channel);
			return std::make_unique<PlayingState>();
		}
	}

	void draw() override {
		gameName.render();
		levelName.render();
	}

	Text gameName, levelName;
	Timer timer;
	int limit, music_channel;
};


std::unique_ptr<State> MenuState::handle_input(){
	switch (menu.handle_input()) {
	case 0:
		return nullptr;
	case 1:
		return std::make_unique<IntroState>();
	case 2:
		return std::make_unique<PlayingState>();
	case 3:
		return std::make_unique<PlayingState>();
	default:
		return std::make_unique<PlayingState>();
	}
}

struct Game
{
	Game(){
		state = std::make_unique<MenuState>();
	}

	bool update() {
		auto new_state = state->update();
		if(new_state != nullptr) {
			state = std::move(new_state);
		}

		return true;
	}
	
	bool handle_input() {
		auto new_state = state->handle_input();
		if (new_state != nullptr) {
			state = std::move(new_state);
		}
		
		return true;
	}

	void draw() {
		state->draw();
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
	float fps = (1.0f / 24.0f)*1000.0f;

	std::vector<GameObject> objects;
	
	bool clicked = false;

	Game game;
	SoundManager sound; // a donde meto esto, no static?

	GameObject g(250,250,64,64,10,10,"tag");

	while (running) {

		SDL_RenderClear(SDL::Context::renderer);

		const Uint8* kbstate = SDL_GetKeyboardState(NULL);
		if (kbstate[SDL_SCANCODE_Q]) {
			running = false;
		}

		if (kbstate[SDL_SCANCODE_L]) {
			camera_x += 32;
		}

		if (kbstate[SDL_SCANCODE_K]) {
			camera_y -= 32;
		}

		if (kbstate[SDL_SCANCODE_J]) {
			camera_x -= 32;
		}

		if (kbstate[SDL_SCANCODE_I]) {
			camera_y += 32;
		}

		if (kbstate[SDL_SCANCODE_T]) {
			camera_angle += 5.0;
		}
		if (kbstate[SDL_SCANCODE_R]) {
			camera_angle -= 5.0;
		}
		
		if (kbstate[SDL_SCANCODE_D] && !pressed) {
			pressed = true;
		}

		while (SDL_PollEvent(&SDL::Context::event)) {
			if (SDL::Context::event.type == SDL_QUIT) {
				running = false;
			}
			if (SDL::Context::event.type == SDL_MOUSEMOTION) {

			}
			if (SDL::Context::event.type == SDL_MOUSEBUTTONDOWN) {
				int x, y;
				SDL_GetMouseState(&x, &y);

				if (!clicked) {
					
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

		LOG("MAIN", "handle_input");
		game.handle_input();
		LOG("MAIN", "update");
		game.update();
		LOG("MAIN", "draw");
		game.draw();

		SDL_SetRenderDrawColor(SDL::Context::renderer, 25, 25, 25, 255);
		SDL_RenderPresent(SDL::Context::renderer);


		int delay = (int)fps - (SDL_GetTicks() - startFTime);
		
		if (delay > 0) SDL_Delay(delay);

		startFTime = SDL_GetTicks();
	}

	return 0;

}