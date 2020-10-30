#pragma once

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <unordered_map>

#include "../MZZEngine.h"

#define DEBUG 0

struct EnemyParticleEmitter : ParticleEmitter {

	EnemyParticleEmitter() : ParticleEmitter(10, 10, 10, 10, 10, 15, 0.0f, -0.05f, 500)
	{
	}

	void push_particle() override {
		if (particles.size() < MAX_PARTICLES) {
			particles.emplace_back(
				Helpers::Random::random_between(x - range_x, x + range_x),
				Helpers::Random::random_between(y - range_y, y + range_y),
				Helpers::Random::random_between(min_w, max_w),
				Helpers::Random::random_between(min_h, max_h),
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
			float _gy = Helpers::Random::random_betweenf(-0.1f, 0.3f);
			float _gx = Helpers::Random::random_betweenf(-0.1f, 0.1f);
			Helpers::Random::random_between(0, 2) == 0 ? _gx = 0.0f : _gy = 0.0f;
			particles.emplace_back(
				Helpers::Random::random_between(x - range_x, x + range_x),
				Helpers::Random::random_between(y - range_y, y + range_y),
				Helpers::Random::random_between(min_w, max_w),
				Helpers::Random::random_between(min_h, max_h),
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

///////////////////
//////STATES///////
///////////////////

struct MenuState : State
{
	MenuState() : menu{ (SDL::Context::screen_w / 2) - 100, (SDL::Context::screen_h / 2) - 45 * 3, 100, 45 * 3, 7 }
	{
	}

	std::unique_ptr<State> update() override {
		return nullptr;
	}

	std::unique_ptr<State> handle_input() override;

	void draw() override {
		menu.draw();
	}

	Menu menu;
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
			if (gameobject.dst_camera.x > SDL::Context::screen_w - 64) {
				Camera::camera_x -= 32;
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
				Camera::camera_x += 32;
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
				Camera::camera_y += 32;
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

			if (gameobject.dst_camera.y > SDL::Context::screen_h - 64) {
				Camera::camera_y -= 32;
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
		target{ target },
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
		v{ 32 }, vx{ v }, vy{ v }, walk_interval{ 1200 }{

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
	EnemyMoveRandomlyAI() : v{ 4 }, vx{ v }, vy{ v } {}

	void update(GameObject& gameobject) override {
		gameobject.dst.x += vx;
		gameobject.dst.y += vy;

		if (gameobject.dst.y > SDL::Context::screen_h) {
			vy = -v;
		}
		if (gameobject.dst.y < 0) {
			vy = v;
		}
		if (gameobject.dst.x > SDL::Context::screen_w) {
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

		if (gameobject.dst.y > SDL::Context::screen_h) {
			vy = -v;
		}
		if (gameobject.dst.y < 0) {
			vy = v;
		}
		if (gameobject.dst.x > SDL::Context::screen_w) {
			vx = -v;
		}
		if (gameobject.dst.y < 0) {
			vx = v;
		}
		if (walking_timer.getTicks() >= walk_interval) {
			gameobject.dst.x += vx;
			gameobject.dst.y += vy;
			if (Helpers::Random::random_between(0, 7) == 0) vx = -v;
			if (Helpers::Random::random_between(0, 7) == 0) vy = -v;

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
		v{ 10 },
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

		if (orientation == SDL_FLIP_NONE) {
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

struct BoomerangBulletAI : AI {
	BoomerangBulletAI(GameObject& player) :
		v{ 10 },
		player{ player }
	{
		orientation = player.flip;
	}

	void update(GameObject& gameobject) override {
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
		enemy_vec{ enemy_vec }, respawn_interval{ respawn_interval }, min{ min }, max{ max }
	{
		respawn_timer.start();
	}

	void add_enemy(int x, int y) {
		auto& e = enemy_vec.emplace_back(x, y, 32, 32, Helpers::Random::random_between(25, 32), 6, "enemy");
		//randomize ai
		e.set_ai(std::make_unique<EnemyMoveRandomlyTimedAI>());
		e.set_emitter(std::make_unique<EnemyParticleEmitter>());
	}

	void update(GameObject& gameobject) override {

		if (respawn_timer.getTicks() > respawn_interval) {
			for (int i = 0; i < Helpers::Random::random_between(min, max); i++) {
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
		: left{ left }, right{ right }, up{ up }, down{ down }
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
		if (a.dst.x < 0 || a.dst.x + a.dst.w > SDL::Context::screen_w ||
			a.dst.x < 0 || a.dst.x + a.dst.w > SDL::Context::screen_w) {
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
		es.set_ai(std::make_unique<EnemySpawnerAI>(enemies, 1000 * 15, 3, 10));
	}

	void add_wall(int x, int y) {
		auto& w = walls.emplace_back(x, y, 32, 32, 0, 1, "wall");
	}

	void add_thing_to_protect(int x, int y) {
		things_to_protect.emplace_back(x, y, 32, 32, 41, 4, "gold");
	}

	void add_reference_tile(int x, int y) {
		reference_tiles.emplace_back(x, y, 32, 32, Helpers::Random::random_between(1, 5), 0, "r_tile");
	}

	void parse_map(std::string file_map_path) {
		std::ifstream stream(file_map_path);
		//if (!stream) LOG("PARSE MAP", "error map file not found\n");
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
						Camera::camera_x = -(player.dst.x);

						player.dst.y = y;
						Camera::camera_y = -(player.dst.y);
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

				x += 32;
			}
			y += 32;
			x = 0;
		}
	}

	void base_update() {

		//fix sword, stop apperieng, somthing with the limit i gues
		//or directly make it boomerang
		for (auto& b : bullets) {
			if (b.ai) b.ai->update(b);
			if (physics.check_boundaries(b, SDL::Context::screen_w * 3)) {
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

	void base_handle_input() {
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
	int enemies_killed, coins_left;
};

struct LostState : State
{
	LostState() :
		msg{ "YOU LOST", SDL::Context::screen_w / 2, SDL::Context::screen_h / 2, 50, {255,125,0,0} }
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
		msg{ "YOU WON", SDL::Context::screen_w / 2, SDL::Context::screen_h / 2, 50, {255,125,0,0} }
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
	LevelOne() : Level{ "demo/levels/level1" } {

		

	}

	std::unique_ptr<State> update() override {
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
		if (level) return level->update();
		//return nullptr;
	}

	std::unique_ptr<State> handle_input() override {
		if (level) return level->handle_input();
		//return nullptr;
	}

	void draw() override {
		if (level) level->draw();
	}

	std::unique_ptr<Level> level;
};

struct IntroState : State
{
	IntroState() :
		gameName{ "A GAME", SDL::Context::screen_w / 2, SDL::Context::screen_h / 2, 50, {255,125,0,0} },
		levelName{ "LEVEL 1: PROTECT THE GOLD", SDL::Context::screen_w / 2, SDL::Context::screen_h / 2, 50, {255,125,0,0} }
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


struct Demo1
{
	Demo1() {
		state = std::make_unique<MenuState>();

		//SOUNDS
		SoundManager::load("wilhelm", "demo/assets/wilhelm.wav");
		SoundManager::load("guitarintro", "demo/assets/guitarintro.wav");
		SoundManager::load("hipintro", "demo/assets/hipintro.wav");
		SoundManager::load("sword", "demo/assets/sword.wav");

	}

	bool update() {
		auto new_state = state->update();
		if (new_state != nullptr) {
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

	std::unique_ptr<State> state{ nullptr };
};
