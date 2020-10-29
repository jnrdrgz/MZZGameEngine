#include "GameObject.h"

GameObject::GameObject(int x, int y, int w, int h, int tilex, int tiley, std::string tag) :
	tile{ tile }, dst{ x,y,w,h }, dst_camera{ x,y,w,h }, src{ tilex * 16,tiley * 16,16,16 },
	angle{ 0.0 }, flip{ SDL_FLIP_NONE }, tag{ tag }, active{ true }{
}

void GameObject::draw() {
	if (active) {
		dst_camera.x = dst.x + Camera::camera_x;
		dst_camera.y = dst.y + Camera::camera_y;
		SDL_RenderCopyEx(SDL::Context::renderer, Assets::Sheet::texture, &src, &dst_camera, angle + Camera::camera_angle, NULL, flip);
	}

	if (emitter) emitter->draw();
}

void GameObject::set_input_handler(std::unique_ptr<InputHandler> ptr) {
	input_handler = std::move(ptr);
}

void GameObject::set_ai(std::unique_ptr<AI> ptr) {
	ai = std::move(ptr);
}

void GameObject::set_animation(std::unique_ptr<Animation> ptr) {
	animation = std::move(ptr);
}

void GameObject::set_emitter(std::unique_ptr<ParticleEmitter> ptr) {
	emitter = std::move(ptr);
}