
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
#include "vector2.h"

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

		load("wilhelm", "wilhelm.wav");
		load("guitarintro", "guitarintro.wav");
		load("hipintro", "hipintro.wav");
		load("sword", "sword.wav");

		//chuncks.push_back(m);
	}

	void load(std::string name, std::string path) {
		auto load_audio = [](std::string file_path) {
			Mix_Chunk* m = Mix_LoadWAV(file_path.c_str());
			if (!m) std::cout << "error loading audio "  << file_path << "\n";
			return m;
		};

		chunks[name] = load_audio(path);
	}

	~SoundManager() {
		for (auto c : chunks) {
			Mix_FreeChunk(c.second);
		}
	}

	static void play_chunk(std::string sound) {
		Mix_PlayChannel(-1, chunks[sound], 0);
	}

	static int play_loop_chunk(std::string sound) {
		return Mix_PlayChannel(-1, chunks[sound], -1);
	}

	static void shut_up_channel(int channel) {
		Mix_HaltChannel(channel);
	}

	static void play_for_seconds(std::string sound, int ms) {
		Mix_PlayChannelTimed(-1, chunks[sound], -1, ms);
	}

	static void play_music(std::string sound) {

	}

	//std::vector<> chuncks;

	static std::unordered_map<std::string, Mix_Chunk*> chunks;
};


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
std::unordered_map<std::string, Mix_Chunk*> SoundManager::chunks;


namespace ParticleManager
{
	////////////////////
	/*
		DELETE PARTICLES WITH AN ITERATOR LIKE IN PHYSICS
	*/
	////////////////////
	struct Particle
	{
	public:
		Particle() {
			life.start();
			active = true;
			gravity = Vector2(0.0f, 0.0f);
			speed = Vector2(0.0f, 0.0f);
			velocity = Vector2(0.0f, 0.0f);
			color = { 255,0,0,255 };
			lifetime = 2000; //ms
			fade = true;
			color_mod = false;
			textured = false;
		}

		Particle(int x, int y, int w, int h, float gy, float wx) : Particle() {
			rct.x = x;
			rct.y = y;
			rct.w = w;
			rct.h = h;

			gravity.y = gy;
			wind.x = wx;
		}

		Particle(int x, int y, int w, int h, float gy, float wx, SDL_Color color) : Particle() {
			rct.x = x;
			rct.y = y;
			rct.w = w;
			rct.h = h;

			gravity.y = gy;
			wind.x = wx;

			this->color = color;
		}

		///textured particle
		Particle(int x, int y, int w, int h, Vector2 gy, Vector2 wn, int tilex, int tiley)
			: rct{ x,y,w,h },
			src{ tilex * 16,tiley * 16, 16,16 },
			gravity{ gy }, wind{ wn }, textured{ true }, active{true} {

			life.start();
			lifetime = 2000; 
			
		}


		void update() {
			time_left = lifetime - life.getTicks();
			if (time_left <= 0) {
				active = false;
				life.stop();
			}

			if (active) {
				if (fade) color.a = (255 * time_left) / lifetime;
				if (color_mod) {
					color.g = (((255 - 130) * time_left) / lifetime) + 130;
				}
				speed += gravity;
				speed += wind;
				speed += velocity;

				if (rct.x >= 500 || rct.x <= 0) { speed.x = 0; }
				if (rct.y >= 400 || rct.y <= 0) { speed.y = 0; }

				rct.x += speed.x;
				rct.y += speed.y;
			}
		}

		void draw() {
			if (textured) {
				//pass angle
				SDL_RenderCopyEx(SDL::Context::renderer, Assets::Sheet::texture, &src, &rct, 0, NULL, SDL_FLIP_NONE);
			}
			else {
				SDL_SetRenderDrawColor(SDL::Context::renderer, color.r, color.g, color.b, color.a);
				SDL_RenderFillRect(SDL::Context::renderer, &rct);
			}
		}

		void set_color(Uint8 r, Uint8 g, Uint8 b) {
			color.r = r;
			color.g = g;
			color.b = b;
		}

		SDL_Rect rct, src;
		Vector2 velocity, speed, gravity, wind;
		float mass;
		Uint32 lifetime;
		int time_left;
		Timer life;
		bool active, fade, color_mod, textured;

		SDL_Color color;
	};

	struct Emitter
	{
	public:
		Emitter() {
			particles.reserve(1000);
		}
		Emitter(Vector2 position) {
			this->position.x = position.x;
			this->position.y = position.y;
			particles.reserve(1000);
			emitting = false;
		}

		void push_particle(int x, int y) {
			int d = random_between(5, 15);
			//Particle p(x, y, d, d, random_betweenf(-0.5f, -0.1f), random_betweenf(-0.1f, 0.04f));
			//p.set_color(255, 255, 0);
			//particles.push_back(p);
			particles.emplace_back(x, y, d, d, random_betweenf(-0.5f, -0.1f), random_betweenf(-0.1f, 0.04f));
		}

		void push_particle() {
			int d = random_between(5, 15);
			//Particle p(position.x, position.y, d, d, random_betweenf(-0.5f, -0.1f), random_betweenf(-0.1f, 0.04f));
			//p.set_color(255, 255, 0);
			//particles.push_back(p);
			particles.emplace_back(position.x, position.y, d, d, random_betweenf(-0.5f, -0.1f), random_betweenf(-0.1f, 0.04f));
		}

		void push_particle(int d, SDL_Color color, bool color_mode = false) {
			//Particle p(position.x, position.y, d, d, particles_g, particles_w);
			//p.set_color(color.r, color.g, color.b);
			//particles.push_back(p);
			//particles.emplace_back(position.x, position.y, d, d, particles_g, particles_w, color);
		}

		void push_particle(int x, int y, int w, int h, Vector2 gvt, Vector2 wnd, int tx, int ty) {
			particles.emplace_back(x, y, w, h, gvt, wnd, tx, ty);
		}

		void emit(int x, int y, int w, int h, Vector2 gvt, Vector2 wnd, int tx, int ty, Uint32 ms) {
			lifetime = ms;
			time.start();
			emitting = true;

			position.x = (float)x;
			position.y = (float)y;

			this->w = w;
			this->h = h;
			this->tx = tx;
			this->ty = ty;
			gravity = gvt;
			wind = wnd;
		}

		void update() {
			if (emitting) {
				wind.x = random_betweenf(-0.4f, 0.4f);
				wind.y = random_betweenf(-0.2f, 0.0f);
				push_particle((int)position.x, (int)position.y, w, h, gravity, wind, tx, ty);
				if (time.getTicks() >= lifetime) {
					time.stop();
					emitting = false;
				}
			}

			std::vector<Particle>::iterator it = particles.begin();
			for (auto& particle : particles) {
				particle.update();

				if (!particle.active) {
					particles.erase(it);
					std::cout << "erased particle\n";
				}
				it++;
			}
		}

		void draw() {
			for (auto& particle : particles) {
				particle.draw();
			}
		}

		Vector2 position;
		Vector2 velocity, speed, gravity, wind;
		int w, h;
		int tx, ty;

		Uint32 lifetime;
		SDL_Color color;
		std::vector<Particle> particles;
		Timer time;
		bool emitting;
	};
}




struct GameObject;

struct InputHandler
{
	virtual void handle_input(GameObject& gameobject) = 0;

	bool pressed[512]{false};
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

struct ParticleEmitter
{
	ParticleEmitter(GameObject& gameobject) : gameobject{ gameobject } {

	}
	virtual void update() = 0;
	virtual void emit() = 0;
	virtual void draw() = 0;

	Timer timer;
	////////////////no esta hecho emitter todavia
	ParticleManager::Emitter emitter;
	GameObject& gameobject;
};



//22 hor
struct GameObject
{
	GameObject(int x, int y, int w, int h, int tilex, int tiley, std::string tag) :
		tile{ tile }, dst{ x,y,w,h }, src{ tilex * 16,tiley * 16,16,16 },
		angle{ 0.0 }, flip{ SDL_FLIP_NONE }, tag{ tag }, active{true}{
	}

	void draw() {
		if (active) SDL_RenderCopyEx(SDL::Context::renderer, Assets::Sheet::texture, &src, &dst, angle, NULL, flip);
		if(emitter) emitter->draw();
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
	SDL_Rect dst, src;
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
		selected{false}
	{
		std::cout << "buttoin created: " <<  x << "-" << y << "-" << w << "-" << h << "-" << margin << "\n";
	}

	void draw() {
		SDL_SetRenderDrawColor(SDL::Context::renderer, 255,0,0,0);
		SDL_RenderFillRect(SDL::Context::renderer, &rct);
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

struct PlayerInputHandler : InputHandler
{
	void hop(GameObject& gameobject) {
		if (!hoping) {
			gameobject.dst.y += 2;
			hoping = true;
		}
		else {
			gameobject.dst.y -= 2;
			hoping = false;
		}
	}

	void handle_input(GameObject& gameobject) {
		const Uint8* kbstate = SDL_GetKeyboardState(NULL);

		//PLAYER MOVEMENT	
		if (kbstate[SDL_SCANCODE_RIGHT] && !pressed[SDL_SCANCODE_RIGHT]) {
			gameobject.dst.x += 32;
			//hop(gameobject);
			gameobject.flip = SDL_FLIP_NONE;
			pressed[SDL_SCANCODE_RIGHT] = true;
		}
		if (!kbstate[SDL_SCANCODE_RIGHT]) {
			pressed[SDL_SCANCODE_RIGHT] = false;
		}

		if (kbstate[SDL_SCANCODE_LEFT] && !pressed[SDL_SCANCODE_LEFT]) {
			gameobject.dst.x -= 32;
			//hop(gameobject);
			gameobject.flip = SDL_FLIP_HORIZONTAL;
			pressed[SDL_SCANCODE_LEFT] = true;
		}
		if (!kbstate[SDL_SCANCODE_LEFT]) {
			pressed[SDL_SCANCODE_LEFT] = false;
		}

		if (kbstate[SDL_SCANCODE_UP] && !pressed[SDL_SCANCODE_UP]) {
			gameobject.dst.y -= 32;
			pressed[SDL_SCANCODE_UP] = true;
		}
		if (!kbstate[SDL_SCANCODE_UP]) {
			pressed[SDL_SCANCODE_UP] = false;
		}

		if (kbstate[SDL_SCANCODE_DOWN] && !pressed[SDL_SCANCODE_DOWN]) {
			gameobject.dst.y += 32;
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

struct NormalBombUpdateAI : AI {
	NormalBombUpdateAI() {
		timer.start();
	}

	void update(GameObject& gameobject) override {

	}

	Timer timer;
};

struct Physics
{
	bool Collision(GameObject& a, GameObject& b) {
		if (a.dst.x < b.dst.x + b.dst.w &&
			a.dst.x + a.dst.w > b.dst.x &&
			a.dst.y < b.dst.y + b.dst.h &&
			a.dst.y + a.dst.h > b.dst.y)
		{
			return true;
		}

		return false;
	}

	bool check_boundaries(GameObject& a) {
		if (a.dst.x < 0 || a.dst.x + a.dst.w > screen_w ||
			a.dst.x < 0 || a.dst.x + a.dst.w > screen_w) {
			return true;
		}

		return false;
	}

	GameObject* Collision(std::vector<GameObject>& group, GameObject& obj) {
		for (auto& g : group) {
			if (Collision(g, obj)) {
				return &g;
			}
		}
		
		return nullptr;
	}

	bool Collision(std::vector<GameObject>& group1, std::vector<GameObject>& group2) {
		for (auto& g1 : group1) {
			for (auto& g2 : group2) {
				if (Collision(g1, g2)) {
					return true;
				}
			}
		}

		return false;
	}


};

///////////////////////////
/// EMITTERs//////////////
/// ////////////////////////

struct EnemyDeathEmitter : ParticleEmitter
{
	EnemyDeathEmitter(GameObject& gameobject) : ParticleEmitter{ gameobject } {

	}

	void update() override {
		emitter.update();
	}

	void emit() override {
		emitter.emit(gameobject.dst.x, gameobject.dst.y, 10, 10, Vector2(0.0f, 0.3f), Vector2(0.0f, 0.0f), 32, 12, 2000);
	}

	void draw() override {
		emitter.draw();
	}
};

struct PlayingState : State
{
	PlayingState() : 
		player{ 32*9,0,32,32,29,0, "player" }
	{
		player.set_input_handler(std::make_unique<PlayerInputHandler>());
		enemy_spawn_timer.start();

		things_to_protect.reserve(13);

		int gold_y = 32;
		int gold_x = 32*9;
		for (int i = 0; i < 13; i++) {
			things_to_protect.emplace_back(gold_x, gold_y, 32, 32, 41, 4, "gold");
			gold_y += 32;
		}

		enemies.reserve(50);
		bullets.reserve(100);
		bombs.reserve(100);

	}
	
	void add_enemy(int x, int y) {
		auto& e = enemies.emplace_back(x, y, 32, 32, random_between(25, 32), 6, "enemy");
		e.set_ai(std::make_unique<EnemyGoToTheGoldAI>(things_to_protect[0]));
		e.set_emitter(std::make_unique<EnemyDeathEmitter>(e));
	}
	
	std::unique_ptr<State> update() override { 
		std::vector<GameObject>::iterator bullets_it = bullets.begin();
		for (auto& b : bullets) {
			if (b.ai) b.ai->update(b);
			
			if (physics.check_boundaries(b)) {
				b.active = false;
			}
			
			if (!b.active) {
				bullets.erase(bullets_it);
			}
			bullets_it++;
		}
		
		std::vector<GameObject>::iterator enemies_it = enemies.begin();
		for (auto& e : enemies) {
			if(e.ai) e.ai->update(e);
			if(e.emitter) e.emitter->update();

			GameObject* obj = physics.Collision(bullets, e);
			if (obj) {
				obj->active = false;
				e.active = false;
				if(e.emitter) e.emitter->emit();
			}

			GameObject* protectable = physics.Collision(things_to_protect, e);
			if (protectable) {
				protectable->active = false;
				e.active = false;
			}

			if (!e.active) {
				enemies.erase(enemies_it);
			}
			enemies_it++;
		}

		if (enemy_spawn_timer.getTicks() > 3000) {
			add_enemy(0,random_between(0,15)*32);
			enemy_spawn_timer.stop();
			enemy_spawn_timer.start();
		}

		return nullptr;
	}

	std::unique_ptr<State> handle_input() override {
		player.input_handler->handle_input(player);

		//my senses say that this is not okay
		PlayerInputHandler* pih = (PlayerInputHandler*)player.input_handler.get();
		if(pih->shooting){
			auto& b = bullets.emplace_back(player.dst.x, player.dst.y, 32, 32, 35, 6, "bullet");
			b.set_ai(std::make_unique<NormalBulletAI>(player));

			pih->shooting = false;
		}

		const Uint8* kbstate = SDL_GetKeyboardState(NULL);
		if (kbstate[SDL_SCANCODE_ESCAPE]) {
			return std::make_unique<MenuState>();
		}

		return nullptr;
	}
	void draw() override {
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
	}

	GameObject player;
	std::vector<GameObject> enemies;
	std::vector<GameObject> things_to_protect;
	std::vector<GameObject> bombs;
	std::vector<GameObject> bullets;
	Timer enemy_spawn_timer;
	Physics physics;

};

struct IntroState : State
{
	IntroState() : intro{ "INTRO", screen_w / 2, screen_h / 2, 100, 50, {255,125,0,0} }
	{
		timer.start();
		limit = 1000 * 10;
		SoundManager::play_for_seconds("hipintro", limit);
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
			return std::make_unique<PlayingState>();
		}
	}

	void draw() override {
		intro.render();
	}

	Text intro;
	Timer timer;
	int limit;
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
	EnemyDeathEmitter emiter(g);

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
					
					clicked = true;
				}
				emiter.emit();
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

		game.handle_input();
		game.update();
		game.draw();

		emiter.update();
		emiter.draw();

		SDL_SetRenderDrawColor(SDL::Context::renderer, 25, 25, 25, 255);
		SDL_RenderPresent(SDL::Context::renderer);


		int delay = (int)fps - (SDL_GetTicks() - startFTime);
		
		if (delay > 0) SDL_Delay(delay);

		startFTime = SDL_GetTicks();
	}

	return 0;

}