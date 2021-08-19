#pragma once

#include "../Math/SimpleMath.cpp"

class Actor {
 private:
	std::vector < std::vector < std::vector < int >>>map;
	int levels, mapx, mapy;
 public:
	double x, y;		// real coords
	double vx, vy;		// Velocity
	double mvx, mvy;	// Max Absolute Velocity
	double fx, fy;		// Friction (on ground)
	double gx, gy;		// Gravity
	double bvx, bvy;	// Min Bounce threshold
	double bfx, bfy;	// Friction (on bounce)
	double mult;		// Multiplier (multiplied to everything)
	 Actor(std::vector < std::vector < std::vector < int >>>map);
	 Actor(double x, double y, std::vector < std::vector < std::vector < int >>>map);
	~Actor();

	// SETUP //
	void setMap(std::vector < std::vector < std::vector < int >>>map);
	void setMultiplier(double multiplier);
	void setMaxVelocity(double x, double y);
	void setGravity(double x, double y);
	void setFriction(double x, double y);
	void setBounceThreshold(double x, double y);
	void setBounceFriction(double x, double y);

	// STATUS //
	bool isOnGround();
	bool isStill();
	bool isTouchingWall();
	bool isTouchingCeiling();
	int xi();
	int yi();
	int level();

	// PHYSICS //
	void addVelocity(double x, double y);

	// CALCULATE //
	void step();
};

Actor::Actor(double x, double y, std::vector < std::vector < std::vector < int >>>map)
{
	setMap(map);
	this->x = x;
	this->y = y;
}

Actor::Actor(std::vector < std::vector < std::vector < int >>>map):Actor::Actor(0, 0, map)
{
}

Actor::~Actor()
{
}

// SETUP //

void Actor::setMap(std::vector < std::vector < std::vector < int >>>map)
{
	this->map = map;
	levels = map.size();
	mapx = map[0].size();
	mapy = map[0][0].size();
}

void Actor::setMultiplier(double multiplier)
{
	mult = multiplier;
}

void Actor::setMaxVelocity(double x, double y)
{
	mvx = x;
	mvy = y;
}

void Actor::setGravity(double x, double y)
{
	gx = x;
	gy = y;
}

void Actor::setFriction(double x, double y)
{
	fx = x;
	fy = y;
}

void Actor::setBounceThreshold(double x, double y)
{
	bvx = x;
	bvy = y;
}

void Actor::setBounceFriction(double x, double y)
{
	bfx = x;
	bfy = y;
}

// STATUS //

bool Actor::isTouchingWall()
{
	int x = xi();
	int y = yi();
	int l = map[level()][x - 1][y];
	int r = map[level()][x + 1][y];
	return (l && !r) || (!l && r) || x == mapx - 2 || x == 1;
}

bool Actor::isTouchingCeiling()
{
	return map[level()][xi()][yi() + 1];
}

bool Actor::isOnGround()
{
	// Only "on ground" when falling onto it
	// if (vy > 0) {
	//      return false;
	// }

	// Check for platform or bottom of world
	if ((int)y <= 1 || map[level()][xi()][yi() - 1]) {
		return true;
	}

	return false;
}

bool Actor::isStill()
{
	return isOnGround() && vx == 0;
}

int Actor::xi()
{
	return (int)x;
}

int Actor::yi()
{
	return (int)y % mapy;
}

int Actor::level()
{
	return ((int)y) / mapy;
}

// PHYSICS //

void Actor::addVelocity(double x, double y)
{
	vx = SimpleMath::clamp(-mvx, mvx, vx + x);
	vy = SimpleMath::clamp(-mvy, mvy, vy + y);
}

// CALCULATE //

void Actor::step()
{
	// Velocity cannot exceed maximum
	vx = SimpleMath::clamp(-mvx, mvx, vx);
	vy = SimpleMath::clamp(-mvy, mvy, vy);

	// Adjust position by velocity and clamp to limits of map
	x = SimpleMath::clamp(1, mapx - 2, x + vx);
	y = SimpleMath::clamp(1, (mapy * levels) - 2, y + vy);

	// Adjust velocity by gravity
	vx += gx;
	vy += gy;

	// Bounce off walls and platforms
	if (isTouchingWall())
		vx = -vx;

	if (isTouchingCeiling())
		vy = -vy;

	// Do ground check AFTER move
	if (isOnGround()) {
		// check for bounce
		if (abs(vy) > bvy) {
			vx = SimpleMath::approach(0, bfx, vx);
			vy = SimpleMath::approach(0, bfy, -vy);	// negative vertical velocity but closer to 0
		} else {
			vy = 0;	// Ground stops you from falling
			vx = SimpleMath::approach(0, fx, vx);	//friction on ground for "sliding"
		}
	}
}
