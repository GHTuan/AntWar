#include "Game.hpp"
#include "Components.hpp"
#include "CustomClasses.hpp"
#include "Global.hpp"
#include "Helper.hpp"
#include "Physic2D.hpp"
#include "SDLCustomEvent.hpp"
#include "Sound.hpp"
#include "Vector2.hpp"
#include "AIController.hpp"

#include <cmath>
#include <iostream>
#include <SDL2/SDL_mixer.h>

SDL_Event Game::event;
GameObject *player = new GameObject("Player");

Game::Game()
{
    isRunning = false;
}

Game::~Game()
{
}

void Game::init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen)
{
    reset = false;

    try
    {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        {
            throw std::runtime_error(std::string("SDL Initialization Failed: ") + SDL_GetError());
        }

        window = SDL_CreateWindow(title, xpos, ypos, width, height, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
        if (!window)
        {
            throw std::runtime_error(std::string("Window Creation Failed: ") + SDL_GetError());
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer)
        {
            throw std::runtime_error(std::string("Renderer Creation Failed: ") + SDL_GetError());
        }
        SDL_SetRenderDrawColor(renderer, 128, 239, 129, 255);

        RENDERER = renderer;

        if (TTF_Init() == -1)
        {
            throw std::runtime_error(std::string("TTF Initialization Failed: ") + TTF_GetError());
        }

        if (Mix_Init(MIX_INIT_MP3) == 0)
        {
            throw std::runtime_error(std::string("Mixer Initialization Failed: ") + Mix_GetError());
        }

        isRunning = true;
        state = MENU;
        objectInit();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Game Initialization Error: " << e.what() << std::endl;
        isRunning = false;
    }
}

void Game::menuSetup()
{
    try
    {
        // 🎮 Tạo menu chính
        Scene *menuScene = new Scene("MainMenu");
        menuScene->AssignLogic([menuScene, this]()
                               {
            Game::state = MENU;
            Sound::GetInstance()->PlayMusic("MenuBgm");

            // 🏞️ Background
            CreateGameObject("Background", Vector2(640, 360), Vector2(1, 1), 
                             "Assets/Sprites/UI/MenuBG.jpg", Vector2(2560, 1707), -10);

            // 🎬 Tiêu đề game
            GameObject *title = CreateGameObject("Title", Vector2(640, 200), Vector2(10, 10), 
                                                 "Assets/Sprites/UI/Game_Name.png", Vector2(64, 16), 0);
            title->AddComponent(new Animator(title, {AnimationClip("Idle", "Assets/Sprites/UI/Game_Name.png", Vector2(64, 16), 200, true, 1.0, 0, 1)}));
            title->GetComponent<Animator>()->Play("Idle");

            // 🕹️ Nút chơi đơn
            CreateButton("PlayButton", Vector2(640, 400), Vector2(5, 5), 
                         "Assets/Sprites/UI/Play_button1p.png", 
                         [this]() { Game::state = GAME; Player2Mode = false; TestMode = false; });

            // 🕹️ Nút chơi đôi
            CreateButton("PlayButtonMulti", Vector2(640, 550), Vector2(5, 5), 
                         "Assets/Sprites/UI/Play_Button2p.png", 
                         [this]() { Game::state = GAME; Player2Mode = true; TestMode = false; });

            // 🔬 Chế độ test (nếu cần)
            if (false) {
                CreateButton("TestButton", Vector2(1100, 700), Vector2(2, 2), 
                             "Assets/Sprites/UI/Play_button.png", 
                             [this]() { Game::state = GAME; Player2Mode = true; TestMode = true; }, 
                             Vector2(32, 16));
            }

            // ❌ Nút thoát (chỉ hiển thị nếu fullscreen)
            if (FULLSCREEN) {
                CreateButton("QuitButton", Vector2(1248, 32), Vector2(2, 2), 
                             "Assets/Sprites/UI/Quit_button.png", 
                             []() {
                                 SDL_Event quitEvent;
                                 quitEvent.type = SDL_QUIT;
                                 SDL_PushEvent(&quitEvent);
                             }, Vector2(32, 32));
            } });

        SceneManager::GetInstance()->AddScene(menuScene);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error setting up menu: " << e.what() << std::endl;
    }
}

void Game::gameOverSetup() {
    try {
        Scene *gameOverScene = new Scene("GameOver");

        gameOverScene->AssignLogic([gameOverScene, this]() {
            Sound::GetInstance()->StopMusic();
            Sound::GetInstance()->PlaySound("Game_Over");

            // Tạo GameObject hiển thị "Game Over"
            CreateGameObject("GameOverText", Vector2(640, 200), Vector2(5, 5),
                             "Assets/Sprites/UI/GameOver.png", Vector2(128, 64), 1);

            // Tạo nút "Menu" để quay lại menu chính
            CreateButton("MenuButton", Vector2(640, 600), Vector2(1.5, 1.5),
                         "Assets/Sprites/UI/menuButton.png",
                         [this]() { Game::state = MENU; scoreTeam1 = scoreTeam2 = 0; },
                         Vector2(143, 48));
        });

        SceneManager::GetInstance()->AddScene(gameOverScene);
    } catch (const std::exception &e) {
        std::cerr << "Error in gameOverSetup: " << e.what() << std::endl;
    }
}
void Game::gameSetup()
{
    Scene *gameScene = new Scene("Game");
    gameScene->AssignLogic([gameScene, this]()
                           {
                               Game::state = GAME;
                               Sound::GetInstance()->PlayMusic("GameBgm");

#pragma region Background Setup
                               GameObject *background = new GameObject("Background");
                               background->transform.position = Vector2(640, 360);
                               background->transform.scale = Vector2(0.128, 0.0960128017068943);

                               background->AddComponent(new SpriteRenderer(background, Vector2(10000, 7499), -10, LoadSpriteSheet("Assets/Sprites/yard.png")));

                               GameObjectManager::GetInstance()->AddGameObject(background);
#pragma endregion

#pragma region Ball Setup
                               GameObject *ball = new GameObject("Ball");
                               ball->tag = 3;
                               ball->transform.position = Vector2(640, 360);
                               ball->transform.scale = Vector2(1.5, 1.5);

                               ball->AddComponent(new SpriteRenderer(ball, Vector2(15, 15), 10, LoadSpriteSheet("Assets/soccer_ball.png")));

                               ball->AddComponent(new Animator(ball, {AnimationClip("Roll", "Assets/soccer_ball.png", Vector2(15, 15), 1000, true, 1.0, 0, 1)}));
                               ball->GetComponent<Animator>()->Play("Roll");

                               ball->AddComponent(new Rigidbody2D(ball, 1, 0.025, .9));

                               ball->AddComponent(new VelocityToAnimSpeedController(ball, "Roll"));
                               ball->AddComponent(new StayInBounds(ball, false));

                               ball->AddComponent(new CircleCollider2D(ball, Vector2(0, 0), 7.5));

                               ball->AddComponent(new BallStateMachine(ball, 2.0, 700, 1000));

                               ball->GetComponent<CircleCollider2D>()->OnCollisionEnter.addHandler(
                                   [ball](Collider2D *collider)
                                   {
                                       ball->GetComponent<BallStateMachine>()->OnCollisionEnter(collider);
                                   });

                               GameObjectManager::GetInstance()->AddGameObject(ball);
#pragma endregion

#pragma region Player setup

                               GameObject *player1 = new GameObject("Player1");
                               player1->transform.scale = Vector2(1.5, 1.5);

                               player1->AddComponent(new SpriteRenderer(player1, Vector2(31, 82), 0, LoadSpriteSheet("Assets/Sprites/ant.png")));
                               player1->AddComponent(new Rigidbody2D(player1, 1, 0.04, 0.5));
                               player1->AddComponent(new CircleCollider2D(player1, Vector2(0, 0), 30 * player1->transform.scale.x));
                               player1->AddComponent(new StayInBounds(player1, false));
                               player1->AddComponent(new RotateTowardVelocity(player1, Vector2(0, -1)));
                               player1->AddComponent(new VelocityToAnimSpeedController(player1, "Run"));

                               player1->transform.position = Vector2(200, HEIGHT / 2);

                               GameObject *player2 = GameObject::Instantiate("Player2", player1, Vector2(300, HEIGHT / 2 + 80), 0, Vector2(1.5, 1.5));
                               GameObject *player3 = GameObject::Instantiate("Player3", player1, Vector2(300, HEIGHT / 2 - 80), 0, Vector2(1.5, 1.5));
                               GameObject *player4 = GameObject::Instantiate("Player4", player1, Vector2(500, HEIGHT / 2 + 150), 0, Vector2(1.5, 1.5));
                               GameObject *player5 = GameObject::Instantiate("Player5", player1, Vector2(500, HEIGHT / 2 - 150), 0, Vector2(1.5, 1.5));

                               GameObject *player6 = GameObject::Instantiate("Player6", player1, Vector2(WIDTH - 500, HEIGHT / 2 + 150), 0, Vector2(1.5, 1.5));
                               GameObject *player7 = GameObject::Instantiate("Player7", player1, Vector2(WIDTH - 500, HEIGHT / 2 - 150), 0, Vector2(1.5, 1.5));
                               GameObject *player8 = GameObject::Instantiate("Player8", player1, Vector2(WIDTH - 300, HEIGHT / 2 + 80), 0, Vector2(1.5, 1.5));
                               GameObject *player9 = GameObject::Instantiate("Player9", player1, Vector2(WIDTH - 300, HEIGHT / 2 - 80), 0, Vector2(1.5, 1.5));
                               GameObject *player10 = GameObject::Instantiate("Player10", player1, Vector2(WIDTH - 200, HEIGHT / 2), 0, Vector2(1.5, 1.5));

                               player1->tag = player2->tag = player3->tag = player4->tag = player5->tag = 1;
                               player6->tag = player7->tag = player8->tag = player9->tag = player10->tag = 2;

                               player1->AddComponent(new Animator(player1, {AnimationClip("Run", "Assets/Sprites/ant.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));
                               player2->AddComponent(new Animator(player2, {AnimationClip("Run", "Assets/Sprites/ant.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));
                               player3->AddComponent(new Animator(player3, {AnimationClip("Run", "Assets/Sprites/ant.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));
                               player4->AddComponent(new Animator(player4, {AnimationClip("Run", "Assets/Sprites/ant.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));
                               player5->AddComponent(new Animator(player5, {AnimationClip("Run", "Assets/Sprites/ant.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));

                               player6->AddComponent(new Animator(player6, {AnimationClip("Run", "Assets/Sprites/ant2.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));
                               player7->AddComponent(new Animator(player7, {AnimationClip("Run", "Assets/Sprites/ant2.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));
                               player8->AddComponent(new Animator(player8, {AnimationClip("Run", "Assets/Sprites/ant2.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));
                               player9->AddComponent(new Animator(player9, {AnimationClip("Run", "Assets/Sprites/ant2.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));
                               player10->AddComponent(new Animator(player10, {AnimationClip("Run", "Assets/Sprites/ant2.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));

                               auto setupCollisionHandler = [](GameObject *player)
                               {
                                   player->GetComponent<CircleCollider2D>()->OnCollisionEnter.addHandler(
                                       [player](Collider2D *collider)
                                       {
                                           if (collider->gameObject->tag == 4)
                                           {
                                               Rigidbody2D *rigidbody = player->GetComponent<Rigidbody2D>();
                                               Sound::GetInstance()->PlaySound("ball_bounce");
                                               rigidbody->BounceOff(collider->GetNormal(player->transform.position));
                                           }
                                       });
                               };

                               setupCollisionHandler(player1);
                               setupCollisionHandler(player2);
                               setupCollisionHandler(player3);
                               setupCollisionHandler(player4);
                               setupCollisionHandler(player5);
                               setupCollisionHandler(player6);
                               setupCollisionHandler(player7);
                               setupCollisionHandler(player8);
                               setupCollisionHandler(player9);
                               setupCollisionHandler(player10);

                               player1->AddComponent(new MovementController(player1, GoalKeeperSpeed, true));
                               player2->AddComponent(new MovementController(player2, DefenderSpeed, true));
                               player3->AddComponent(new MovementController(player3, DefenderSpeed, true));
                               player4->AddComponent(new MovementController(player4, AttackerSpeed, true));
                               player5->AddComponent(new MovementController(player5, AttackerSpeed, true));

                               if (Player2Mode)
                               {
                                   player10->AddComponent(new MovementController(player10, GoalKeeperSpeed, true));
                                   player9->AddComponent(new MovementController(player9, DefenderSpeed, true));
                                   player8->AddComponent(new MovementController(player8, DefenderSpeed, true));
                                   player7->AddComponent(new MovementController(player7, AttackerSpeed, true));
                                   player6->AddComponent(new MovementController(player6, AttackerSpeed, true));
                               }

                               player1->AddComponent(new KickControl(player1, ball, SDLK_SPACE, HIGH_KICK_FORCE));
                               player2->AddComponent(new KickControl(player2, ball, SDLK_SPACE, LOW_KICK_FORCE));
                               player3->AddComponent(new KickControl(player3, ball, SDLK_SPACE, LOW_KICK_FORCE));
                               player4->AddComponent(new KickControl(player4, ball, SDLK_SPACE, HIGH_KICK_FORCE));
                               player5->AddComponent(new KickControl(player5, ball, SDLK_SPACE, HIGH_KICK_FORCE));

                               if (Player2Mode)
                               {
                                   player6->AddComponent(new KickControl(player6, ball, SDLK_KP_ENTER, HIGH_KICK_FORCE));
                                   player7->AddComponent(new KickControl(player7, ball, SDLK_KP_ENTER, HIGH_KICK_FORCE));
                                   player8->AddComponent(new KickControl(player8, ball, SDLK_KP_ENTER, LOW_KICK_FORCE));
                                   player9->AddComponent(new KickControl(player9, ball, SDLK_KP_ENTER, LOW_KICK_FORCE));
                                   player10->AddComponent(new KickControl(player10, ball, SDLK_KP_ENTER, HIGH_KICK_FORCE));
                               }

                               player1->AddComponent(new AIGoalKeeper(player1, ball, GoalKeeperSpeed, true));
                               player2->AddComponent(new AIDefender(player2, ball, DefenderSpeed, true));
                               player3->AddComponent(new AIDefender(player3, ball, DefenderSpeed, true));
                               player4->AddComponent(new AIAttacker(player4, ball, AttackerSpeed, true));
                               player5->AddComponent(new AIAttacker(player5, ball, AttackerSpeed, true));

                               player6->AddComponent(new AIAttacker(player6, ball, AttackerSpeed, false));
                               player7->AddComponent(new AIAttacker(player7, ball, AttackerSpeed, false));
                               player8->AddComponent(new AIDefender(player8, ball, DefenderSpeed, false));
                               player9->AddComponent(new AIDefender(player9, ball, DefenderSpeed, false));
                               player10->AddComponent(new AIGoalKeeper(player10, ball, GoalKeeperSpeed, false));

                               GameObject *controllerSwitcher1 = new GameObject("ControllerSwitcher1");
                               TeamControl *movementControllerSwitcher1 = dynamic_cast<TeamControl *>(controllerSwitcher1->AddComponent(
                                   new TeamControl(controllerSwitcher1, LoadSpriteSheet("Assets/blue_indicator.png"), 75.0)));
                               movementControllerSwitcher1->AddMovementController(SDLK_1, player1->GetComponent<MovementController>());
                               movementControllerSwitcher1->AddMovementController(SDLK_2, player2->GetComponent<MovementController>());
                               movementControllerSwitcher1->AddMovementController(SDLK_3, player3->GetComponent<MovementController>());
                               movementControllerSwitcher1->AddMovementController(SDLK_4, player4->GetComponent<MovementController>());
                               movementControllerSwitcher1->AddMovementController(SDLK_5, player5->GetComponent<MovementController>());
                               GameObjectManager::GetInstance()->AddGameObject(controllerSwitcher1);

                               if (Player2Mode || TestMode)
                               {
                                   GameObject *controllerSwitcher2 = new GameObject("ControllerSwitcher2");
                                   TeamControl *movementControllerSwitcher2 = dynamic_cast<TeamControl *>(controllerSwitcher2->AddComponent(
                                       new TeamControl(controllerSwitcher2, LoadSpriteSheet("Assets/red_indicator.png"), 75.0)));
                                   movementControllerSwitcher2->AddMovementController(SDLK_KP_6, player6->GetComponent<MovementController>());
                                   movementControllerSwitcher2->AddMovementController(SDLK_KP_7, player7->GetComponent<MovementController>());
                                   movementControllerSwitcher2->AddMovementController(SDLK_KP_8, player8->GetComponent<MovementController>());
                                   movementControllerSwitcher2->AddMovementController(SDLK_KP_9, player9->GetComponent<MovementController>());
                                   movementControllerSwitcher2->AddMovementController(SDLK_KP_0, player10->GetComponent<MovementController>());
                                   GameObjectManager::GetInstance()->AddGameObject(controllerSwitcher2);
                               }

                               GameObjectManager::GetInstance()->AddGameObject(player1);

                               GameObjectManager::GetInstance()->AddGameObject(player10);

                               if (!TestMode)
                               {
                                   GameObjectManager::GetInstance()->AddGameObject(player2);
                                   GameObjectManager::GetInstance()->AddGameObject(player3);
                                   GameObjectManager::GetInstance()->AddGameObject(player4);
                                   GameObjectManager::GetInstance()->AddGameObject(player5);
                                   GameObjectManager::GetInstance()->AddGameObject(player6);
                                   GameObjectManager::GetInstance()->AddGameObject(player7);
                                   GameObjectManager::GetInstance()->AddGameObject(player8);
                                   GameObjectManager::GetInstance()->AddGameObject(player9);
                               }
                               else
                               {
                                   player2->GetComponent<Collider2D>()->enabled = false;
                                   player3->GetComponent<Collider2D>()->enabled = false;
                                   player4->GetComponent<Collider2D>()->enabled = false;
                                   player5->GetComponent<Collider2D>()->enabled = false;
                                   player6->GetComponent<Collider2D>()->enabled = false;
                                   player7->GetComponent<Collider2D>()->enabled = false;
                                   player8->GetComponent<Collider2D>()->enabled = false;
                                   player9->GetComponent<Collider2D>()->enabled = false;
                               }

#pragma endregion

#pragma region Goal Setup
                               GameObject *goal1 = new GameObject("Goal1");
                               goal1->transform.position = Vector2(90, 360);
                               goal1->transform.scale = Vector2(1.5, 0.8);
                               goal1->tag = 5;

                               goal1->AddComponent(new SpriteRenderer(goal1, Vector2(72, 200), 0, LoadSpriteSheet("Assets/Sprites/goal0.png")));
                               goal1->AddComponent(new BoxCollider2D(goal1, Vector2(0, 0),
                                                                     Vector2(72 * goal1->transform.scale.x, 176 * goal1->transform.scale.y)));

                               goal1->GetComponent<BoxCollider2D>()->OnCollisionEnter.addHandler(
                                   [goal1, this](Collider2D *collider)
                                   {
                                       BoxCollider2D *goal1Col = goal1->GetComponent<BoxCollider2D>();
                                       if (collider->gameObject->tag == 3)
                                       {
                                           if (goal1Col->GetNormal(collider->gameObject->transform.position) == Vector2(1, 0))
                                           {
                                               Sound::GetInstance()->PlaySound("Goal");
                                               this->scoreTeam1++;
                                               SceneManager::GetInstance()->LoadScene("Game");
                                           }
                                           else
                                           {
                                               Rigidbody2D *rigidbody = collider->gameObject->GetComponent<Rigidbody2D>();
                                               rigidbody->BounceOff(goal1Col->GetNormal(collider->gameObject->transform.position));
                                           }
                                       }
                                       else
                                       {
                                           Rigidbody2D *rigidbody = collider->gameObject->GetComponent<Rigidbody2D>();
                                           collider->gameObject->transform.position += goal1Col->GetNormal(collider->gameObject->transform.position) * rigidbody->velocity.Magnitude();
                                       }
                                   });

                               GameObjectManager::GetInstance()->AddGameObject(goal1);

                               GameObject *goal2 = new GameObject("Goal2");
                               goal2->transform.position = Vector2(1190, 360);
                               goal2->transform.rotation = 180;
                               goal2->transform.scale = Vector2(1.5, 0.8);
                               goal2->tag = 7;

                               goal2->AddComponent(new SpriteRenderer(goal2, Vector2(72, 200), 0, LoadSpriteSheet("Assets/Sprites/goal0.png")));
                               goal2->AddComponent(new BoxCollider2D(goal2, Vector2(0, 0),
                                                                     Vector2(72 * goal2->transform.scale.x, 176 * goal2->transform.scale.y)));

                               goal2->GetComponent<BoxCollider2D>()->OnCollisionEnter.addHandler(
                                   [goal2, this](Collider2D *collider)
                                   {
                                       BoxCollider2D *goal2Col = goal2->GetComponent<BoxCollider2D>();
                                       if (collider->gameObject->tag == 3)
                                       {
                                           if (goal2Col->GetNormal(collider->gameObject->transform.position) == Vector2(-1, 0))
                                           {
                                               Sound::GetInstance()->PlaySound("Goal");
                                               this->scoreTeam2++;
                                               SceneManager::GetInstance()->LoadScene("Game");
                                               return;
                                           }
                                           else
                                           {
                                               Rigidbody2D *rigidbody = collider->gameObject->GetComponent<Rigidbody2D>();
                                               rigidbody->BounceOff(goal2Col->GetNormal(collider->gameObject->transform.position));
                                           }
                                       }
                                       else
                                       {
                                           Rigidbody2D *rigidbody = collider->gameObject->GetComponent<Rigidbody2D>();
                                           collider->gameObject->transform.position += goal2Col->GetNormal(collider->gameObject->transform.position) * rigidbody->velocity.Magnitude();
                                       }
                                   });

                               GameObjectManager::GetInstance()->AddGameObject(goal2);

#pragma endregion
                           });

    SceneManager::GetInstance()->AddScene(gameScene);
}

void Game::objectInit()
{

    // Add sounds and music
    Sound::GetInstance();
    Sound::GetInstance()->AddMusic("MenuBgm", "Assets/SFX/fairyfountain.mp3", 100);
    Sound::GetInstance()->AddMusic("GameBgm", "Assets/SFX/papyrus.mp3", 32);

    Sound::GetInstance()->AddSound("ball_bounce", "Assets/SFX/ball_bounce.mp3", 128);
    Sound::GetInstance()->AddSound("ball_kick", "Assets/SFX/ball_kick.mp3", 128);

    Sound::GetInstance()->AddSound("Game_Over", "Assets/SFX/gameover.mp3", 128);
    Sound::GetInstance()->AddSound("Goal", "Assets/SFX/score.mp3", 64);

    std::cout << "Object Initialisation..." << std::endl;

    menuSetup();

    gameOverSetup();

    gameSetup();

    SceneManager::GetInstance()->LoadScene("MainMenu");
}

void Game::handleEvents()
{

    SDL_PollEvent(&Game::event);

    if (event.type == SDL_QUIT)
    {
        isRunning = false;
        return;
    }

    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.sym == SDLK_ESCAPE)
        {
            state = MENU;
            scoreTeam1 = scoreTeam2 = 0;
            return;
        }
    }

    // End condition
    if (scoreTeam1 + scoreTeam2 >= 5)
    {
        state = GAMEOVER;
        return;
    }
}

void Game::handleSceneChange()
{
    switch (state)
    {
    case MENU:
        if (SceneManager::GetInstance()->GetCurrentScene()->GetName() != "MainMenu")
            SceneManager::GetInstance()->LoadScene("MainMenu");
        break;
    case GAME:
        if (SceneManager::GetInstance()->GetCurrentScene()->GetName() != "Game")
            SceneManager::GetInstance()->LoadScene("Game");
        break;
    case GAMEOVER:
        if (SceneManager::GetInstance()->GetCurrentScene()->GetName() != "GameOver")
            SceneManager::GetInstance()->LoadScene("GameOver");
        break;
    }
}

void Game::update()
{
    SceneManager::GetInstance()->Update();
}

void Game::render()
{
    SDL_RenderClear(renderer);
    SceneManager::GetInstance()->Draw();

    // Show score
    if (state == GAME)
    {
        SDL_Color textColor = {0, 0, 0, 255};
        std::string scoreText = std::to_string(scoreTeam1) + " - " + std::to_string(scoreTeam2);
        SDL_Texture *scoreTexture = LoadFontTexture(scoreText, "Assets/Fonts/arial.ttf", textColor, 50);
        if (scoreTexture)
        {
            RenderTexture(scoreTexture, 640, 20);
            SDL_DestroyTexture(scoreTexture);
        }
        else
        {
            std::cerr << "Failed to load score texture" << std::endl;
        }
    }

    if (state == GAMEOVER)
    {
        // Render final scores
        SDL_Color textColor = {255, 255, 255, 255};
        std::string scoreText = "Final Score: " + std::to_string(scoreTeam1) + " - " + std::to_string(scoreTeam2);
        SDL_Texture *scoreTexture = LoadFontTexture(scoreText, "Assets/Fonts/arial.ttf", textColor, 75);
        if (scoreTexture)
        {
            RenderTexture(scoreTexture, 640, 400); // Centered below "Game Over!"
            SDL_DestroyTexture(scoreTexture);
        }
        else
        {
            std::cerr << "Failed to load score texture" << std::endl;
        }
    }

    SDL_RenderPresent(renderer);
}

void Game::clean()
{
    delete SceneManager::GetInstance();

    for (auto &texture : TEXTURES)
    {
        SDL_DestroyTexture(texture);
    }
    TEXTURES.clear();

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);

    TTF_Quit();
    SDL_Quit();
}

bool Game::running()
{
    return isRunning;
}

bool Game::reseting()
{
    return reset;
}

GameObject *Game::CreateGameObject(const std::string &name, Vector2 position, Vector2 scale,
                                   const std::string &spritePath, Vector2 spriteSize, int drawOrder)
{
    try
    {
        GameObject *obj = new GameObject(name);
        obj->transform.position = position;
        obj->transform.scale = scale;
        obj->AddComponent(new SpriteRenderer(obj, spriteSize, drawOrder, LoadSpriteSheet(spritePath)));
        GameObjectManager::GetInstance()->AddGameObject(obj);
        return obj;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error creating object " << name << ": " << e.what() << std::endl;
        return nullptr;
    }
}

void Game::CreateButton(const std::string &name, Vector2 position, Vector2 scale,
                        const std::string &spritePath, std::function<void()> onClick,
                        Vector2 spriteSize)
{
    try
    {
        GameObject *button = CreateGameObject(name, position, scale, spritePath, spriteSize, 0);
        button->AddComponent(new BoxCollider2D(button, Vector2(0, 0), Vector2(spriteSize.x * scale.x, spriteSize.y * scale.y)));
        button->AddComponent(new Button(button));
        button->GetComponent<Button>()->AddOnClickHandler(onClick);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error creating button " << name << ": " << e.what() << std::endl;
    }
}
