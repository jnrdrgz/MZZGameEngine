#pragma once

#include <memory>

struct State
{
	virtual std::unique_ptr<State> update() = 0;
	virtual std::unique_ptr<State> handle_input() = 0;
	virtual void draw() = 0;
};