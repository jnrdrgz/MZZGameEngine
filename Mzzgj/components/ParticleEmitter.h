#pragma once
#include <SDL.h>
#include "../SDL_Context.h"
#include "../Helpers.h"
#include <vector>
#include "../timer.h"
#include "../AssetsManager.h"

struct Particle
{
	Particle(int x, int y, int w, int h, int tx, int ty, Uint32 lifetime,
		float gx, float gy);

	void update();

	//here_goes
	void draw();

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
	ParticleEmitter();

	ParticleEmitter(int range_x, int range_y,
		int min_w, int max_w, int min_h, int max_h,
		int tx, int ty, float gx, float gy, Uint32 lifetime);

	ParticleEmitter(int range_x, int range_y,
		int max_size, int min_size,
		int tx, int ty, float gx, float gy, Uint32 lifetime);


	void emit(int x, int y, Uint32 emitter_lifetime);

	virtual void push_particle() = 0;

	virtual void update() = 0;

	void draw();

	Uint32 lifetime;
	Timer life;
	Uint32 emitter_lifetime{ 0 };
	std::vector<Particle> particles;
	const size_t MAX_PARTICLES;

	int x, y, range_x, range_y, min_w, max_w, min_h, max_h, tx, ty;
	float gx, gy;
	bool emitting;
	bool finished;
};