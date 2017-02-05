#include "level.h"
#include "graphics.h"
#include "globals.h"
#include "tinyxml2.h"
#include "utils.h"
#include "slope.h"

#include<SDL.h>
#include <sstream>
#include <algorithm>
#include <cmath>

struct Tileset;

using namespace tinyxml2;

Level::Level() {}

Level::Level(std::string mapName, Graphics &graphics) :
	_mapName(mapName),
	_size(Vector2(0, 0))

{
	this->loadMap(mapName, graphics);
}

Level::~Level() {

}

void Level::loadMap(std::string mapName, Graphics &graphics) {
	//Parsing the tmx file
	XMLDocument doc;
	std::stringstream ss;
	ss << "content/maps/" << mapName << ".tmx";
	doc.LoadFile(ss.str().c_str());

	XMLElement* mapNode = doc.FirstChildElement("map");

	//getting width and height of map- store in _size
	int width;
	int height;
	mapNode->QueryIntAttribute("width", &width);
	mapNode->QueryIntAttribute("height", &height);
	this->_size = Vector2(width, height);

	//get width and height of tiles- store in _tileSize
	int tileWidth;
	int tileHeight;
	mapNode->QueryIntAttribute("tilewidth", &tileWidth);
	mapNode->QueryIntAttribute("tileheight", &tileHeight);
	this->_tileSize = Vector2(tileWidth, tileHeight);

	//loading tilesets
	XMLElement* pTileset = mapNode->FirstChildElement("tileset");
	if (pTileset != NULL) {
		while (pTileset) {
			int firstgid;
			const char* source = pTileset->FirstChildElement("image")->Attribute("source");
			char* path;
			std::stringstream ss;
			ss << source;
			pTileset->QueryIntAttribute("firstgid", &firstgid);
			SDL_Texture* tex = SDL_CreateTextureFromSurface(graphics.getRenderer(), graphics.loadImage(ss.str()));
			this->_tilesets.push_back( Tileset(tex, firstgid));

			//get all the animations for that tileset
			XMLElement* pTileA = pTileset->FirstChildElement("tile");
			if (pTileA != NULL) {
				while (pTileA) {
					AnimatedTileInfo ati;
					ati.StartTileId = pTileA->IntAttribute("id") + firstgid;
					ati.TilesetsFirstGid = firstgid;
					XMLElement* pAnimation = pTileA->FirstChildElement("animation");
					if (pAnimation != NULL) {
						while (pAnimation) {
							XMLElement* pFrame = pAnimation->FirstChildElement("frame");
							if (pFrame != NULL) {
								while (pFrame) {
									ati.TileIds.push_back(pFrame->IntAttribute("tileid") + firstgid);
									ati.Duration = pFrame->IntAttribute("duration");
									pFrame = pFrame->NextSiblingElement("frame");
								}
							}
							pAnimation = pAnimation->NextSiblingElement("animation");
						}
					}
					this->_animatedTileInfos.push_back(ati);
					pTileA = pTileA->NextSiblingElement("tile");
				}
			}
			pTileset = pTileset->NextSiblingElement("tileset");
		}
	}
	//loading layers`
	XMLElement* pLayer = mapNode->FirstChildElement("layer");
	if (pLayer != NULL) {
		while (pLayer) {
			//loading the data element
			XMLElement* pData = pLayer->FirstChildElement("data");
			if (pData != NULL) {
				while (pData) {
					//loading the tile element
					XMLElement* pTile = pData->FirstChildElement("tile");
					if (pTile != NULL) {
						int tileCounter = 0;
						while (pTile) {
							//build each individual tile
							//if gid = 0 , dont draw anything
							if (pTile->IntAttribute("gid") == 0) {
								tileCounter++;
								if (pTile->NextSiblingElement("tile")) {
									pTile = pTile->NextSiblingElement("tile");
									continue;
								}
								else {
									break;
								}
							}
							//if gid != 0
							//get the tileset for this specific gid
							int gid = pTile->IntAttribute("gid");
							Tileset tls;
							int closest = 0;
							for (int i = 0; i < this->_tilesets.size(); i++) {
								if (this->_tilesets[i].FirstGid <= gid) {
									if (this->_tilesets[i].FirstGid > closest) {
										closest = this->_tilesets[i].FirstGid;
										tls = this->_tilesets.at(i);
									}
								}
							}	
							if (tls.FirstGid == -1) {
								//no tileset found for this gid
								tileCounter++;
								if (pTile->NextSiblingElement("tile")) {
									pTile = pTile->NextSiblingElement("tile");
									continue;
								}
								else {
									break;
								}
							}
							
							//get position of tile and level
							int xx = 0;
							int yy = 0;
							xx = tileCounter % width;
							xx *= tileWidth;
							yy += tileHeight * (tileCounter / width);
							Vector2 finalTilePosition = Vector2(xx, yy);

							//Calculate the position of the tileset
							Vector2 finalTilesetPosition = this->getTilesetPosition(tls, gid, tileWidth, tileHeight);

							//build tile and add it to the levels tiles list
							bool isAnimateTile = false;
							AnimatedTileInfo ati;
							for (int i = 0; i < this->_animatedTileInfos.size(); i++) {
								if (this->_animatedTileInfos.at(i).StartTileId == gid) {
									ati = this->_animatedTileInfos.at(i);
									isAnimateTile = true;
									break;
								}
							}
							if (isAnimateTile == true) {
								std::vector<Vector2> tilesetPositions;
								for (int i = 0; i < ati.TileIds.size(); i++) {
									tilesetPositions.push_back(this->getTilesetPosition(tls, ati.TileIds.at(i), tileWidth, tileHeight));
								}
									AnimatedTile tile(tilesetPositions, ati.Duration, tls.Texture, Vector2(tileWidth, tileHeight), finalTilePosition);
									this->_animatedTileList.push_back(tile);
							}
							else {
								Tile tile(tls.Texture, Vector2(tileWidth, tileHeight), finalTilesetPosition, finalTilePosition);
								this->_tileList.push_back(tile);
							}
							tileCounter++;
							pTile = pTile->NextSiblingElement("tile");
						}
					}
					pData = pData->NextSiblingElement("data");
				}
			}
			pLayer = pLayer->NextSiblingElement("layer");
		}
	}

	//parsing collisions
	XMLElement* pObjectGroup = mapNode->FirstChildElement("objectgroup");
	if (pObjectGroup != NULL) {
		while (pObjectGroup) {
			const char* name = pObjectGroup->Attribute("name");
			std::stringstream ss;
			ss << name;
			if (ss.str() == "collisions") {
				XMLElement* pObject = pObjectGroup->FirstChildElement("object");
				if (pObject != NULL) {
					while (pObject) {
						float x, y, width, height;
						x = pObject->FloatAttribute("x");
						y = pObject->FloatAttribute("y");
						width = pObject->FloatAttribute("width");
						height = pObject->FloatAttribute("height");
						this->_collisionRects.push_back(Rectangle(
							std::ceil(x) * globals::SPRITE_SCALE,
							std::ceil(y) * globals::SPRITE_SCALE,
							std::ceil(width) * globals::SPRITE_SCALE,
							std::ceil(height) * globals::SPRITE_SCALE
							));

						pObject = pObject->NextSiblingElement("object");
					}
				}
			}

			//other onject group go here with else if(ss.str = "nameofotherobjectgroup"
			else if (ss.str() == "slopes") {
				XMLElement* pObject = pObjectGroup->FirstChildElement("object");
				if (pObject != NULL) {
					while (pObject) {
						std::vector<Vector2> points;
						Vector2 p1;
						p1 = Vector2(std::ceil(pObject->FloatAttribute("x")), std::ceil(pObject->FloatAttribute("y")));

						XMLElement* pPolyline = pObject->FirstChildElement("polyline");
						if (pPolyline != NULL) {
							std::vector<std::string> pairs;
							const char* pointString = pPolyline->Attribute("points");

							std::stringstream ss;
							ss << pointString;
							Utils::split(ss.str(), pairs, ' ');
							//now we have each of the pairs. loop through list of pairs and split into Vector2
							//store in points vector
							for (int i = 0; i < pairs.size(); i++) {
								std::vector<std::string> ps;
								Utils::split(pairs.at(i), ps, ',');
								points.push_back(Vector2(std::stoi(ps.at(0)), std::stoi(ps.at(1))));
							}
						}

						for (int i = 0; i < points.size(); i += 2) {
							this->_slopes.push_back(Slope(
							Vector2((p1.x + points.at(i < 2 ? i : i - 1).x) * globals::SPRITE_SCALE,
									(p1.y + points.at(i < 2 ? i : i - 1).y) * globals::SPRITE_SCALE),
							Vector2((p1.x + points.at(i < 2 ? i + 1 : i).x) * globals::SPRITE_SCALE,
									(p1.y + points.at(i < 2 ? i + 1 : i).y) * globals::SPRITE_SCALE)
								));
						}

						pObject = pObject->NextSiblingElement("object");
					}
				}
			}
			else if (ss.str() == "spawn points") {
				XMLElement* pObject = pObjectGroup->FirstChildElement("object");
				if (pObject != NULL) {
					while (pObject) {
						float x = pObject->FloatAttribute("x");
						float y = pObject->FloatAttribute("y");
						const char* name = pObject->Attribute("name");
						std::stringstream ss;
						ss << name;
						if (ss.str() == "player") {
							this->_spawnPoint = Vector2(std::ceil(x) * globals::SPRITE_SCALE, std::ceil(y) * globals::SPRITE_SCALE);
						}

						pObject = pObject->NextSiblingElement("object");
					}
				}
			}

			else if (ss.str() == "doors") {
				XMLElement* pObject = pObjectGroup->FirstChildElement("object");
				if (pObject != NULL) {
					while (pObject) {
						float x = pObject->FloatAttribute("x");
						float y = pObject->FloatAttribute("y");
						float w = pObject->FloatAttribute("width");
						float h = pObject->FloatAttribute("height");
						Rectangle rect = Rectangle(x, y, w, h);

						XMLElement* pProperties = pObject->FirstChildElement("properties");
						if (pProperties != NULL) {
							while (pProperties) {
								XMLElement* pProperty = pProperties->FirstChildElement("property");
								if (pProperty != NULL) {
									while (pProperty) {
										const char* name = pProperty->Attribute("name");
										std::stringstream ss;
										ss << name;
										if (ss.str() == "destination") {
											const char* value = pProperty->Attribute("value");
											std::stringstream ss2;
											ss2 << value;
											Door door = Door(rect, ss2.str());
											this->_doorList.push_back(door);
										}
										pProperty = pProperty->NextSiblingElement("property");
									}
								}
								pProperties = pProperties->NextSiblingElement("properties");
							}
						}

						pObject = pObject->NextSiblingElement("object");
					}
				}
			}



			pObjectGroup = pObjectGroup->NextSiblingElement("objectgroup");
		}
	}
}

void Level::update(int elapsedTime) {
	for (int i = 0; i < this->_animatedTileList.size(); i++) {
		this->_animatedTileList.at(i).update(elapsedTime);
	}

}


void Level::draw(Graphics &graphics) {
	for (int i = 0;i < this->_tileList.size(); i++) {
		this->_tileList.at(i).draw(graphics);
	}
	for (int i = 0; i < this->_animatedTileList.size(); i++) {
		this->_animatedTileList.at(i).draw(graphics);
	}
}

std::vector<Rectangle> Level::checkTileCollisions(const Rectangle &other) {
	std::vector<Rectangle> others;
	for (int i = 0; i < this->_collisionRects.size(); i++) {
		if (this->_collisionRects.at(i).collidesWith(other)) {
			others.push_back(this->_collisionRects.at(i));
		}
	}
	return others;
}

std::vector<Slope> Level::checkSlopeCollisions(const Rectangle &other) {
	std::vector<Slope> others;
	for (int i = 0; i < this->_slopes.size(); i++){
		if (this->_slopes.at(i).collidesWith(other)) {
			others.push_back(this->_slopes.at(i));
		}
	}
	return others;
}

std::vector<Door> Level::checkDoorCollisions(const Rectangle &other) {
	std::vector<Door> others;
	for (int i = 0; i < this->_doorList.size(); i++) {
		if (this->_doorList.at(i).collidesWith(other)) {
			others.push_back(this->_doorList.at(i));
		}
	}
	return others;
}

const Vector2 Level::getPlayerSpawnPoint() const {
	return this->_spawnPoint;
}

Vector2 Level::getTilesetPosition(Tileset tls, int gid, int tileWidth, int tileHeight) {
	int tilesetWidth, tilesetHeight;
	SDL_QueryTexture(tls.Texture, NULL, NULL, &tilesetWidth, &tilesetHeight);
	int tsxx = (gid - 1) % (tilesetWidth / tileWidth) ;
	tsxx *= tileWidth;
	int tsyy = 0;
	int amt = ((gid - tls.FirstGid) / (tilesetWidth / tileWidth));
	tsyy = tileHeight * amt;
	Vector2 finalTilesetPosition = Vector2(tsxx, tsyy);
	
	return finalTilesetPosition;
}