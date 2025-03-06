#ifndef GAME_HPP
#define GAME_HPP
#include "Object.hpp"
#include "Vector2.hpp"

#include <SDL2/SDL.h>
#include <functional>
#include <string>
#include <set>
#include <random>

class Game
{
public:
    Game();
    ~Game();

    enum State
    {
        MENU,
        GAME,
        GAMEOVER
    };

    State state = MENU;

    void init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen);
    void menuSetup();
    void gameOverSetup();
    void gameSetup();
    void objectInit();

    void handleEvents();
    void handleSceneChange();
    void update();
    void render();
    void clean();

    bool running();
    bool reseting();

    GameObject *CreateGameObject(const std::string &name, Vector2 position, Vector2 scale,
                                 const std::string &spritePath, Vector2 spriteSize, int drawOrder);

    void CreateButton(const std::string &name, Vector2 position, Vector2 scale,
                      const std::string &spritePath, std::function<void()> onClick,
                      Vector2 spriteSize = Vector2(48, 16));

    static SDL_Event event;

    int scoreTeam1 = 0;
    int scoreTeam2 = 0;

private:
    bool isRunning;
    bool reset = false;
    SDL_Window *window;
    SDL_Renderer *renderer;
};

int GetUniqueRandom(std::set<int> &usedValues, int minVal, int maxVal);
#endif // GAME_HPP
