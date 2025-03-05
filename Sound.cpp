#include "Sound.hpp"

Sound *Sound::instance = nullptr;

#pragma region Sound

// Sound class implementation
Sound::Sound() {
    try {
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
            throw std::runtime_error(std::string("SDL_mixer could not initialize! Error: ") + Mix_GetError());
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

Sound::~Sound() {
    try {
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
    } catch (const std::exception &e) {
        std::cerr << "Error in Sound destructor: " << e.what() << std::endl;
    }
}

void Sound::AddMusic(const std::string &name, const std::string &path, int volume) {
    try {
        Mix_Music *newMusic = Mix_LoadMUS(path.c_str());
        if (!newMusic) {
            throw std::runtime_error(std::string("Failed to load music: ") + path + " Error: " + Mix_GetError());
        }
        music[name] = newMusic;
        musicVolumes[name] = volume;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

void Sound::PlayMusic(const std::string &name, int loops) {
    try {
        auto it = music.find(name);
        if (it == music.end()) {
            throw std::runtime_error(std::string("Music not found: ") + name);
        }
        
        if (currentMusic != name) {
            currentMusic = name;
            Mix_VolumeMusic(musicVolumes[name]);
            if (Mix_PlayMusic(it->second, loops) < 0) {
                throw std::runtime_error(std::string("Failed to play music: ") + name + " Error: " + Mix_GetError());
            }
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

void Sound::StopMusic() {
    Mix_HaltMusic();
}

void Sound::PauseMusic() {
    Mix_PauseMusic();
}

void Sound::ResumeMusic() {
    Mix_ResumeMusic();
}

Sound* Sound::GetInstance() {
    if (instance == nullptr) {
        instance = new Sound();
    }
    return instance;
}

void Sound::AddSound(const std::string &name, const std::string &path, int volume) {
    try {
        Mix_Chunk *newSound = Mix_LoadWAV(path.c_str());
        if (!newSound) {
            throw std::runtime_error(std::string("Failed to load sound: ") + path + " Error: " + Mix_GetError());
        }
        sounds[name] = newSound;
        soundVolumes[name] = volume;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

void Sound::PlaySound(const std::string &name, int loops) {
    try {
        auto it = sounds.find(name);
        if (it == sounds.end()) {
            throw std::runtime_error(std::string("Sound not found: ") + name);
        }
        
        Mix_Volume(-1, soundVolumes[name]);
        Mix_PlayChannel(-1, it->second, loops);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}


void Sound::StopSound() {
    Mix_HaltChannel(-1);
}

void Sound::PauseSound() {
    Mix_Pause(-1);
}

void Sound::ResumeSound() {
    Mix_Resume(-1);
}



#pragma endregion
