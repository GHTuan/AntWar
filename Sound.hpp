#ifndef SOUND_HPP
#define SOUND_HPP

#include <iostream>
#include <map>
#include <SDL2/SDL_mixer.h>

class SoundManager {
    private:
        std::map<std::string, Mix_Music *> music;
        std::map<std::string, Mix_Chunk *> sounds;
    
        std::map<std::string, int> musicVolumes;
        std::map<std::string, int> soundVolumes;
    
        std::string currentMusic;
    
        SoundManager();
        static SoundManager *instance;
    public:
        ~SoundManager();
        static SoundManager *GetInstance();
    
        void AddMusic(std::string name, std::string path, int volume);
        void AddSound(std::string name, std::string path, int volume);
    
        void PlayMusic(std::string name, int loops = -1);
        void PlaySound(std::string name, int loops = 0);
    
        void StopMusic();
        void StopSound();
    
        void PauseMusic();
        void PauseSound();
    
        void ResumeMusic();
        void ResumeSound();
    
    };

#endif // SOUND_HPP 