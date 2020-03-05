
#pragma once

#include "State.h"

class Game;

class StubState1
	: public State
{
public:
	StubState1()
		: State()
	{}

	~StubState1() override {};

	void Init(Game* game) override { 
		gamePtr = game;
	};

	void HandleInput(SDL_Event* event) override {};
	void Update(float deltaTime) override {};
	void Render(float alpha = 1.0f) override {

	};

	void RenderUI() {};
	void Cleanup() override {};

	Game* gamePtr;
};
