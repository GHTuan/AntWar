#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "CustomClasses.hpp"
#include "Game.hpp"
#include "Helper.hpp"
#include "Physic2D.hpp"
#include "Sound.hpp"

#include "cmath"


class BallStateMachine : public Component {
private:
    float maxSpeed = 0;
    float kickedStateMinSpeed = 0;

    GameObject *lastKickedBy = nullptr;
    float bounceKickerCooldown = 0;

    float lastKickedTime = 0;

    GameObject *lastBindedBy = nullptr;             //Check binded
    Vector2 lastBindedVelocity = Vector2(0, 0);     //Check binded

    float bindCooldown = 0;                         //Check binded
    float lastBindTime = 0;                         //Check binded

public:
    enum State {
        FREE,
        BINDED,                                     //Check binded
        KICKED
    };

    BallStateMachine(GameObject *parent, float kickedStateMinSpeed, float bindCoolDown, float bounceKickerCooldown) : Component(parent) {
        currentState = FREE;
        this->kickedStateMinSpeed = kickedStateMinSpeed;
        this->bindCooldown = bindCoolDown;
        this->bounceKickerCooldown = bounceKickerCooldown;

        this->rigidbody = this->gameObject->GetComponent<Rigidbody2D>();
    }

    void OnCollisionEnter(Collider2D *other) {
        if (currentState == FREE) {
            if (other->gameObject->tag == 1 || other->gameObject->tag == 2) {
                Bind(other->gameObject, true);
            }
        } else if (currentState == BINDED && other->gameObject->tag != lastBindedBy->tag) {
            // Bind to a player with a different tag than the last player it was bound to
            Bind(other->gameObject);
        } else if (currentState == KICKED) {
            // If collided with the last kicker
            // Check if the cooldown has passed
            if (other->gameObject == lastKickedBy && SDL_GetTicks() - lastKickedTime > bounceKickerCooldown) {
                Bind(other->gameObject);
            }

            // If collided with a teammate of the last player it was bound to
            else if (other->gameObject->tag == lastBindedBy->tag) {
                Bind(other->gameObject);
            }

            // Bounce off anything else
            else {
                Sound::GetInstance()->PlaySound("ball_bounce");
                rigidbody->BounceOff(other->GetNormal(gameObject->transform.position));
            }
        }
    }

    void Update() {
        if (currentState == FREE) {
        } else if (currentState == BINDED) {
            if (lastBindedBy != nullptr) {
                Rigidbody2D *boundRigidbody = lastBindedBy->GetComponent<Rigidbody2D>();
                CircleCollider2D *boundCollider = lastBindedBy->GetComponent<CircleCollider2D>();

                if (boundRigidbody == nullptr || boundCollider == nullptr) {
                    currentState = FREE;
                    return;
                }

                if (boundRigidbody->velocity.Magnitude() > 0.1f) {
                    lastBindedVelocity = boundRigidbody->velocity;
                }

                gameObject->transform.position = lastBindedBy->transform.position + lastBindedVelocity.Normalize() * boundCollider->radius;
            }
        } else if (currentState == KICKED) {
            if (rigidbody->velocity.Magnitude() < kickedStateMinSpeed) {
                currentState = FREE;
            }
        }
    }

    void Kick(Vector2 direction, float force, GameObject *kicker) {
        if (currentState == BINDED) {
            Sound::GetInstance()->PlaySound("ball_kick");
            currentState = KICKED;
            rigidbody->AddForce(direction * force);
            lastKickedBy = kicker;
            lastKickedTime = SDL_GetTicks();

            // Set bindcooldown;
            lastBindTime = SDL_GetTicks();

            // Set backup rigidbody
            gameObject->GetComponent<VelocityToAnimSpeedController>()->SetBackupRigidbody(nullptr);
        }
    }

    void Bind(GameObject *binder, bool ignoreCooldown = false) {
        if (SDL_GetTicks() - lastBindTime < bindCooldown && !ignoreCooldown)
            return;

        lastBindTime = SDL_GetTicks();
        currentState = BINDED;
        lastBindedBy = binder;

        rigidbody->velocity = Vector2(0, 0);

        gameObject->GetComponent<VelocityToAnimSpeedController>()->SetBackupRigidbody(binder->GetComponent<Rigidbody2D>());
    }

    void Draw() {}

    GameObject *GetBinded() {
        if (currentState == BINDED)
            return lastBindedBy;
        else
            return nullptr;
    }

    Component *Clone(GameObject *parent) {
        BallStateMachine *newBallStateMachine = new BallStateMachine(parent, kickedStateMinSpeed, bindCooldown, bounceKickerCooldown);
        return newBallStateMachine;
    }

private:
    State currentState;
    Rigidbody2D *rigidbody;
};

// Player Control
class MovementController : public Component {

private:
    Rigidbody2D *rigidbody;
    SDL_Keycode upKey, downKey, leftKey, rightKey;

    float upSpeed = 0, downSpeed = 0, leftSpeed = 0, rightSpeed = 0;

public:
    float speed = 0;

    MovementController(GameObject *parent, float speed, bool useWASD) : Component(parent) {
        this->speed = speed;
        this->rigidbody = this->gameObject->GetComponent<Rigidbody2D>();

        if (useWASD) {
            upKey = SDLK_w;
            downKey = SDLK_s;
            leftKey = SDLK_a;
            rightKey = SDLK_d;
        } else {
            upKey = SDLK_UP;
            downKey = SDLK_DOWN;
            leftKey = SDLK_LEFT;
            rightKey = SDLK_RIGHT;
        }
    }

    void Update() {
        if (!enabled)
            return;
        if (rigidbody == nullptr)
            return;

        float actualSpeed = speed * 1 / FPS;

        if (Game::event.type == SDL_KEYDOWN || Game::event.type == SDL_KEYUP) {
            if (Game::event.key.keysym.sym == upKey) {
                upSpeed = Game::event.type == SDL_KEYDOWN ? -1 : 0;
            }
            if (Game::event.key.keysym.sym == downKey) {
                downSpeed = Game::event.type == SDL_KEYDOWN ? 1 : 0;
            }
            if (Game::event.key.keysym.sym == leftKey) {
                leftSpeed = Game::event.type == SDL_KEYDOWN ? -1 : 0;
            }
            if (Game::event.key.keysym.sym == rightKey) {
                rightSpeed = Game::event.type == SDL_KEYDOWN ? 1 : 0;
            }
        }

        rigidbody->AddForce(Vector2(leftSpeed + rightSpeed, upSpeed + downSpeed).Normalize() * actualSpeed);
    }

    void Enable() {
        ClearSpeed();
        enabled = true;
    }
    void Disable() {
        ClearSpeed();
        enabled = false;
    }
    bool GetEnabled() {
        return enabled;
    }

    void ClearSpeed() {
        upSpeed = 0;
        downSpeed = 0;
        leftSpeed = 0;
        rightSpeed = 0;
    }

    void Draw() {}

    Component *Clone(GameObject *parent) {
        MovementController *newMovementController = new MovementController(parent, speed, upKey == SDLK_w);
        return newMovementController;
    }
};
class TeamControl : public Component {
private:
    SDL_Texture *indicator = nullptr;
    float indicatorRadius = 0;

    std::map<int, MovementController *> movementControllers;
    int currentKey = -1;

public:
    TeamControl(GameObject *parent, SDL_Texture *indicator, float indicatorRadius) : Component(parent) {
        this->indicator = indicator;
        this->indicatorRadius = indicatorRadius;
    }

    void Update() {
        if (Game::event.type == SDL_KEYDOWN) {
            SDL_Keycode key = Game::event.key.keysym.sym;
            if (movementControllers.find(key) != movementControllers.end()) {
                movementControllers[currentKey]->Disable();

                movementControllers[key]->Enable();
                currentKey = key;
            }
        }
    }

    void Draw() {
        if (currentKey != -1) {
            MovementController *currentController = movementControllers[currentKey];
            GameObject *currentObject = currentController->gameObject;
            SpriteRenderer *spriteRenderer = currentObject->GetComponent<SpriteRenderer>();

            if (spriteRenderer != nullptr) {
                SDL_Rect rect = {
                    static_cast<int>(currentObject->transform.position.x - indicatorRadius / 2),
                    static_cast<int>(currentObject->transform.position.y - indicatorRadius / 2),
                    static_cast<int>(indicatorRadius),
                    static_cast<int>(indicatorRadius)};
                SDL_RenderCopy(RENDERER, indicator, nullptr, &rect);
            }
        }
    }

    void AddMovementController(int keyBind, MovementController *movementController) {
        movementControllers[keyBind] = movementController;

        // Enable only the first movement controller when adding
        if (movementControllers.size() > 1) {
            movementController->Disable();
        } else {
            // set the current key to the first key binded
            currentKey = keyBind;
        }
    }

    void RemoveMovementController(int keyBind) {
        movementControllers.erase(keyBind);

        // Enable the first controller when removing
        if (movementControllers.find(keyBind) != movementControllers.end()) {
            movementControllers.begin()->second->Enable();
        }
    }

    Component *Clone(GameObject *parent) {
        TeamControl *newMovementControllerSwitcher = new TeamControl(parent, indicator, indicatorRadius);
        for (auto &movementController : movementControllers) {
            newMovementControllerSwitcher->AddMovementController(movementController.first, movementController.second);
        }
        return newMovementControllerSwitcher;
    }
};

class KickControl : public Component {
private:
    Rigidbody2D *rigidbody = nullptr;
    SDL_KeyCode kickKey = SDLK_SPACE;

    GameObject *ball = nullptr;
    BallStateMachine *ballStateMachine = nullptr;

    float kickForce;

    Vector2 lastDirection = Vector2(0, 0);

public:
    KickControl(GameObject *parent, GameObject *ball, SDL_KeyCode kickKey, float kickForce) : Component(parent) {
        this->rigidbody = this->gameObject->GetComponent<Rigidbody2D>();
        this->ball = ball;
        this->kickKey = kickKey;

        this->kickForce = kickForce;

        ballStateMachine = ball->GetComponent<BallStateMachine>();
    }

    void Update() {
        if (!rigidbody || !ballStateMachine) {
            rigidbody = gameObject->GetComponent<Rigidbody2D>();
            ballStateMachine = ball->GetComponent<BallStateMachine>();
            if (!rigidbody || !ballStateMachine) {
                return;
            }
        }

        auto movementController = gameObject->GetComponent<MovementController>();
        if (!movementController || !movementController->GetEnabled()) {
            return;
        }

        if (Game::event.type == SDL_KEYDOWN && Game::event.key.keysym.sym == kickKey) {
            if (ballStateMachine->GetBinded() != gameObject) {
                return;
            }
            if (rigidbody->velocity.Magnitude() > 0.01f) {
                lastDirection = rigidbody->velocity.Normalize();
            }
            ballStateMachine->Kick(lastDirection, kickForce, gameObject);
        }
    }

    void Draw() {}

    Component *Clone(GameObject *parent) {
        KickControl *newShootControl = new KickControl(parent, ball, kickKey, kickForce);
        return newShootControl;
    }
};



class Button : public Component {
private:
    Collider2D *collider = nullptr;

    Event<> *onClick = nullptr;

public:
    Button(GameObject *parent) : Component(parent) {
        onClick = new Event<>();
    }

    ~Button() {
        delete onClick;
    }

    void Update() {
        if (collider == nullptr) {
            collider = gameObject->GetComponent<Collider2D>();
            if (collider == nullptr)
                return;
        }

        if (Game::event.type == SDL_MOUSEBUTTONDOWN) {
            Vector2 mousePosition = Vector2(Game::event.button.x, Game::event.button.y);
            if (collider->CheckCollision(mousePosition)) {
                this->onClick->raise();
            }
        }
    }

    void Draw() {}

    void AddOnClickHandler(std::function<void()> handler) {
        onClick->addHandler(handler);
    }

    Component *Clone(GameObject *parent) {
        Button *newButton = new Button(parent);
        newButton->onClick = onClick;
        return newButton;
    }
};

#endif
