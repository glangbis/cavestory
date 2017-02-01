#ifndef ANIMATED_SPRITE_H
#define ANIMATED_SPRITE_H

#include  <map>
#include <string>
#include <vector>
#include "globals.h"
#include "sprite.h"

class Graphics;

//animated sprite class, holds logic for animated sprites

class AnimatedSprite : public Sprite {
public:
	AnimatedSprite();
	AnimatedSprite(Graphics &graphics, const std::string &filePath, int sourceX, int sourceY, int width, int height, float posX, float posY, float timeToUpdate);

	//plays animation provided it is not already playing
	void playAnimation(std::string animation, bool once = false);

	//updates animated sprite timer
	void update(int elapsedTime);

	//draws sprite to game
	void draw(Graphics &graphics, int x, int y);



protected:
	double _timeToUpdate;
	bool _currentAnimationOnce ;
	std::string _currentAnimation;

	//adds animation to map of animation for the sprite
	void addAnimation(int frames, int x, int y, std::string name, int width, int height, Vector2 offset);

	//reset animation of sprite
	void resetAnimations();

	//stops current animation
	void stopAnimation();

	//changes visibility of animates sprite
	void setVisible(bool visible);

	//logic when an animation ends
	virtual void animationDone(std::string currentAnimation) = 0;

	//required for setting up animation
	virtual void setupAnimations() = 0;

private:
	std::map < std::string, std::vector<SDL_Rect>> _animations;
	std::map<std::string, Vector2 > _offsets;

	int _frameIndex ;
	double _timeElapsed ;
	bool _visible;


};

#endif