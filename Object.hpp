#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <iostream>

#include <functional>
#include <map>
#include <SDL2/SDL.h>

#include "Vector2.hpp"

class GameObject;

// Event
template <typename... Args>
class Event
{
public:
    using Handler = std::function<void(Args...)>;

    Event() {}
    ~Event() { handlers.clear(); }

    void addHandler(Handler handler)
    {
        handlers.push_back(handler);
    }

    void raise(Args... args)
    {
        for (auto &handler : handlers)
        {
            handler(std::forward<Args>(args)...);
        }
    }

private:
    std::vector<Handler> handlers;
};

// Specialization for no arguments
template <>
class Event<>
{
public:
    using Handler = std::function<void()>;

    Event() {}
    ~Event() { handlers.clear(); }

    void addHandler(Handler handler)
    {
        handlers.push_back(handler);
    }

    void raise()
    {
        for (auto &handler : handlers)
        {
            handler();
        }
    }

private:
    std::vector<Handler> handlers;
};

/*Singleton manager for GameObjects, automatic memory management
 */
class GameObjectManager
{
private:
    std::map<std::string, GameObject *> gameObjects;
    GameObjectManager();
    static GameObjectManager *instance;

public:
    ~GameObjectManager();

    static GameObjectManager *GetInstance();
    void AddGameObject(std::vector<GameObject *> gameObjects);
    void AddGameObject(GameObject *gameObject);
    void RemoveGameObject(std::string name);
    GameObject *GetGameObject(std::string name);
    std::vector<GameObject *> GetGameObjectsByTag(int tag);
    void Clear();

    void Update();
    void Draw();
};

class Component
{
public:
    bool enabled = true;

    GameObject *gameObject = nullptr;
    Component(GameObject *parent);
    virtual ~Component();
    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual Component *Clone(GameObject *parent) = 0;
};

SDL_Texture *LoadSpriteSheet(std::string path);

class SpriteRenderer : public Component
{
private:
    int drawOrder = 0;

public:
    SDL_Texture *spriteSheet = nullptr;
    SDL_Rect spriteRect;

    bool isFlipped;

    // static void SetRenderer(SDL_Renderer *renderer);

    SpriteRenderer(GameObject *gameObject, Vector2 spriteSize, int drawOrder = 0, SDL_Texture *defaultSpriteSheet = nullptr);
    ~SpriteRenderer();
    void Update();
    void Draw();
    Component *Clone(GameObject *parent);

    int GetDrawOrder();
    void SetTexture(SDL_Texture *newTexture)
    {
        spriteSheet = newTexture;
    }
};

class AnimationClip
{
private:
    SDL_Texture *spriteSheet;
    SDL_Rect currentSpriteRect;
    std::string name;
    float length = 0;

    // Only used in cloning, by another AnimationClip

public:
    bool loop = false, isPlaying = false;

    Vector2 spriteSize = Vector2(1, 1);

    int currentSprite = 0, startSprite = 0, endSprite = 0;

    float speedScale = 0, animCooldown = 0, lastFrameTime = 0, startTime = 0;

    Event<> *onComplete = nullptr;

    AnimationClip();
    AnimationClip(std::string name, std::string path, Vector2 spriteSize, float length, bool loop, float speedScale, int startSprite, int endSprite);
    AnimationClip(const AnimationClip &clip);

    ~AnimationClip();

    std::string GetName();
    void AdvanceFrame();
    void Ready();
    std::pair<SDL_Texture *, SDL_Rect> GetCurrentSpriteInfo();
};

class Animator : public Component
{
private:
    std::map<std::string, AnimationClip> clips;
    AnimationClip *currentClip = nullptr;

public:
    Animator(GameObject *gameObject, std::vector<AnimationClip> clips);
    ~Animator();

    // Update the SpriteRenderer with the current sprite
    void Update();
    void Draw();

    void Play(std::string name);
    void Stop();

    AnimationClip *GetCurrentClip();
    AnimationClip *GetClip(std::string name);
    std::vector<AnimationClip> GetAllClips();

    Component *Clone(GameObject *parent);
};

class Transform
{
public:
    float rotation; // Only for the Z axis
    Vector2 position, scale;
    Transform();
    Transform(Vector2 position, float rotation, Vector2 scale);
};

class GameObject
{
private:
    std::string name;
    std::vector<Component *> components;

public:
    Transform transform;
    int tag = 0;

    GameObject();
    GameObject(std::string name);
    ~GameObject();
    void Update();
    void Draw();

    std::string GetName();

    Component *AddComponent(Component *component);

    template <typename T>
    T *GetComponent();

    static GameObject *Instantiate(std::string name, const GameObject *origin, Vector2 position, float rotation, Vector2 scale);
    static void Destroy(std::string name);
};

template <typename T>
T *GameObject::GetComponent()
{
    for (auto &component : components)
    {
        if (dynamic_cast<T *>(component))
        {
            return dynamic_cast<T *>(component);
        }
    }
    return nullptr;
}

// More like a template for the GameObjectManager

// Wrapper for all, including GameObjectManager
// Singleton

class Scene
{
private:
    std::string name;

    std::function<void()> logic;

public:
    Scene(std::string name);
    ~Scene();

    void AssignLogic(std::function<void()> logic);
    void RunLogic();

    void Load();

    std::string GetName();
};
class SceneManager
{
private:
    Scene *currentScene;
    SceneManager();
    static SceneManager *instance;

    std::map<std::string, Scene *> scenes;

public:
    ~SceneManager();
    static SceneManager *GetInstance();

    void RunLogic();

    void AddGameObject(GameObject *gameObject);
    void RemoveGameObject(std::string name);
    GameObject *GetGameObject(std::string name);

    void AddScene(Scene *scene);
    void LoadScene(std::string sceneName);
    Scene *GetCurrentScene();

    void Update();
    void Draw();
};

#endif
