#include "demo1.h"

std::unique_ptr<State> MenuState::handle_input() {
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