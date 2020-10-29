#include "ParticleEmitter.h"
#include "../Camera.h"

Particle::Particle(int x, int y, int w, int h, int tx, int ty, Uint32 lifetime,
	float gx, float gy) :
	dst{ x,y,w,h },
	src{ 16 * tx, 16 * ty, 16,16 },
	dst_camera{ x + Camera::camera_x,y + Camera::camera_y,w,h },
	tx{ tx }, ty{ ty },
	gx{ gx }, gy{ gy }, active{ true }, lifetime{ lifetime }
{
	angle = 0.0f;
	life.start();
}

void Particle::update() {
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
void Particle::draw() {
	if (active) {
		dst_camera.x = dst.x + Camera::camera_x;
		dst_camera.y = dst.y + Camera::camera_y;
		SDL_RenderCopyEx(SDL::Context::renderer, Assets::Sheet::texture, &src, &dst_camera, angle, NULL, SDL_FLIP_NONE);

	}
}

ParticleEmitter::ParticleEmitter() : MAX_PARTICLES{ 200 } {
	particles.reserve(MAX_PARTICLES);
}

ParticleEmitter::ParticleEmitter(int range_x, int range_y,
	int min_w, int max_w, int min_h, int max_h,
	int tx, int ty, float gx, float gy, Uint32 lifetime)
	: range_x{ range_x }, range_y{ range_y },
	min_w{ min_w }, max_w{ max_w }, min_h{ min_h }, max_h{ max_h },
	tx{ tx }, ty{ ty }, gx{ gx }, gy{ gy }, lifetime{ lifetime }, finished{ false },
	MAX_PARTICLES{ 200 } {


	particles.reserve(MAX_PARTICLES);
	emitting = false;
}

ParticleEmitter::ParticleEmitter(int range_x, int range_y,
	int max_size, int min_size,
	int tx, int ty, float gx, float gy, Uint32 lifetime)
	: range_x{ range_x }, range_y{ range_y },
	min_w{ min_size }, max_w{ max_size }, min_h{ min_size }, max_h{ max_size },
	tx{ tx }, ty{ ty }, gx{ gx }, gy{ gy }, lifetime{ lifetime }, finished{ false },
	MAX_PARTICLES{ 200 } {


	particles.reserve(MAX_PARTICLES);
	emitting = false;
}


void ParticleEmitter::emit(int x, int y, Uint32 emitter_lifetime) {
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

void ParticleEmitter::draw() {
	for (auto& p : particles) {
		p.draw();
	}
}