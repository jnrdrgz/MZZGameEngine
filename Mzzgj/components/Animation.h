#pragma once

#include "../timer.h"

#include <memory>

struct GameObject;

struct Animation
{
	virtual void update(GameObject& gameobject) = 0;

	Timer timer;
};