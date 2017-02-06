#ifndef PLAYER_H
#define PLAYER_H


#include "animatedsprite.h"
#include "globals.h"
#include "slope.h"
#include "level.h"
class Graphics;

class Player : public AnimatedSprite {
public:
	Player();
	Player(Graphics &graphics, Vector2 spawnPoint);
	void draw(Graphics &graphics);
	void update(float elapsedTime);

	//moves player left by -dx
	void moveLeft();

	//moves player right by +dx
	void moveRight();

	//stop moving the player, keep current direction, play idle animation
	void stopMoving();

		
	void lookUp();
	void stopLookingUp();
	//player looks down or interacts with objects(turn around)
	void lookDown();
	void stopLookingDown();

	//jump
	void jump();

	//dash
	void dash();




	virtual void animationDone(std::string currentAnimation);
	virtual void setupAnimations();

	void handleTileCollisions(std::vector<Rectangle> &others);
	void handleSlopeCollisions(std::vector<Slope> &others);
	void handleDoorCollision(std::vector<Door> &others, Level &level, Graphics &graphics);
	void handleBelowMap(Level &level);

	const float getX() const;
	const float getY() const;

	const inline int getMaxHealth() { return _maxHealth; }
	const inline int getCurrentHealth() { return _currentHealth; }

private:
	float _dx, _dy;

	Direction _facing; 

	bool _grounded;

	bool _lookingUp;
	bool _lookingDown;

	int _maxHealth;
	int _currentHealth;

};

#endif