#pragma once
#include "IRuntimeModule.hpp"
#include "Vector.h"
#include <vector>

struct MassPoint {
	Game::Vector2 position;
	Game::Vector2 velocity;
	Game::Vector2 accel;

	float mass;

	MassPoint(Game::Vector2 position, float mass) :
		position(position), mass(mass), velocity(),
		accel() {}
};

struct Spring {
	int p0index, p1index;

	float originLength;

	Spring(int p1, int p2,float maxLength
	,float minLength,float dis) :
		p0index(p1), p1index(p2){
		originLength = dis;
		originLength = maxLength < originLength ? maxLength : originLength;
		originLength = minLength > originLength ? minLength : originLength;
		currentForce = 0.;
	}

	Spring& operator=(const Spring& o) {
		p0index = o.p0index, p1index = o.p1index,
		originLength = o.originLength,currentForce = o.currentForce;
		return *this;
	}

	//how heavy the spring can take
	const float maxForce = 10.;
	float currentForce;
};


class SpringSys : public Game::IRuntimeModule {
public:
	virtual bool initialize() override;
	virtual void tick() override;
	virtual void finalize() override;
private:

	void iter(float dt);
	void drawPoints();
	void initScene();

	Game::Vector2 cursorPosition();


	const float gravity = 1.0;
	const float ground = -1.8;
	const float maxSpringLength = 1.5;
	const float minSpringLength = 0.1;
	float stillness = 2e3;
	float damp = 1e-1;

	const float lineWidth = 0.05;
	const float pointSize = 0.12;

	float recordLine;
	float maxHeight;

	Game::Vector2 viewCenter;
	float viewHeight;

	int limitation;

	std::vector<MassPoint> massPoints;
	std::vector<Spring> springs;
};