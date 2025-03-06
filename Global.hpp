#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <SDL2/SDL.h>
#include <vector>

extern SDL_Renderer *RENDERER;
extern std::vector<SDL_Texture *> TEXTURES;

const int FPS = 60;
const int WIDTH = 1280, HEIGHT = 720;
const int MIN_WIDTH_YARD = 155, MIN_HEIGHT_YARD = 95;
const int MAX_WIDTH_YARD = 1125, MAX_HEIGHT_YARD = 625;
const bool FULLSCREEN = false;

const float HIGH_KICK_FORCE = 17.0f;
const float LOW_KICK_FORCE = 12.0f;

const float GoalKeeperSpeed = 15.0f;
const float DefenderSpeed = 10.0f;
const float AttackerSpeed = 11.0f;

static bool Player2Mode = false;
static bool TestMode = false;

#define EPS 0.0001

#endif // GLOBALS_HPP