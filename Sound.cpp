#include "Sound.hpp"

SoundManager *SoundManager::instance = nullptr;

#pragma region SoundManager

// SoundManager class implementation
SoundManager::SoundManager() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
    }
}

SoundManager::~SoundManager() {
    for (auto &pair : music) {
        Mix_FreeMusic(pair.second);
    }
    music.clear();

    for (auto &pair : sounds) {
        Mix_FreeChunk(pair.second);
    }
    sounds.clear();

    Mix_Quit();

    instance = nullptr;
}

SoundManager* SoundManager::GetInstance() {
    if (instance == nullptr) {
        instance = new SoundManager();
    }
    return instance;
}

void SoundManager::AddMusic(std::string name, std::string path, int volume = 128) {
    Mix_Music *newMusic = Mix_LoadMUS(path.c_str());
    if (!newMusic) {
        std::cerr << "Failed to load music: " << path << " SDL_mixer Error: " << Mix_GetError() << std::endl;
        return;
    }
    music[name] = newMusic;
    musicVolumes[name] = volume;
}

void SoundManager::AddSound(std::string name, std::string path, int volume = 128) {
    Mix_Chunk *newSound = Mix_LoadWAV(path.c_str());
    if (!newSound) {
        std::cerr << "Failed to load sound: " << path << " SDL_mixer Error: " << Mix_GetError() << std::endl;
        return;
    }
    sounds[name] = newSound;
    soundVolumes[name] = volume;
}

void SoundManager::PlayMusic(std::string name, int loops) {
    auto it = music.find(name);
    if (it != music.end()) {
        if (currentMusic == name) {
            return;
        }
        
        currentMusic = name;
        Mix_VolumeMusic(musicVolumes[name]);
        Mix_PlayMusic(it->second, loops);
    } else {
        std::cerr << "Music not found: " << name << std::endl;
    }
}

void SoundManager::PlaySound(std::string name, int loops) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {

        Mix_Volume(-1, soundVolumes[name]);
        Mix_PlayChannel(-1, it->second, loops);
        
    } else {
        std::cerr << "Sound not found: " << name << std::endl;
    }
}

void SoundManager::StopMusic() {
    Mix_HaltMusic();
}

void SoundManager::StopSound() {
    Mix_HaltChannel(-1);
}

void SoundManager::PauseMusic() {
    Mix_PauseMusic();
}

void SoundManager::PauseSound() {
    Mix_Pause(-1);
}

void SoundManager::ResumeMusic() {
    Mix_ResumeMusic();
}

void SoundManager::ResumeSound() {
    Mix_Resume(-1);
}



#pragma endregion
