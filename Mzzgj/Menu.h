#pragma once

#include <SDL.h>
#include <vector>
#include "Button.h"

struct Menu
{
	Menu(int x, int y, int w, int h, int padding) {
		buttons.reserve(3);

		int bh = (h / 3);
		int by = y;
		buttons.emplace_back(x, by, w, bh, "PLAY", padding);
		by += bh + padding;
		buttons.emplace_back(x, by, w, bh, "OPTIONS", padding);
		by += bh + padding;
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