#ifndef GRAPHICS_H
#define GRAPHICS_H

//Graphics for the game

#include <map>
#include <string>
#include <SDL.h>





class Graphics {
public:
	Graphics();
	~Graphics();

	//loads an image into the _spriteSheets map if it doesnt already exist
	//this makes all the image to be loaded only once
	//returns the image from the map regardless of whether or not it was loaded
	SDL_Surface* loadImage(const std::string &filePath);

	//draws texture to a certain part of screen
	void blitSurface(SDL_Texture* texture, SDL_Rect* sourceRectangle, SDL_Rect* destinationRectangle);

	//Renders everything to screen
	void flip();

	//clears the screen
	void clear();

	//returns renderer
	SDL_Renderer* getRenderer() const;

private:
	SDL_Window* _window;
	SDL_Renderer* _renderer;

	std::map<std::string, SDL_Surface*> _spriteSheets;
};

#endif
