#pragma once

struct GameObject;

struct AI
{
	virtual void update(GameObject& gameobject) = 0;
};