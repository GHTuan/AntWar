#ifndef SOUND_HPP
#define SOUND_HPP

#include <iostream>
#include <map>
#include <SDL2/SDL_mixer.h>

class Sound {
    private:
        std::map<std::string, Mix_Music *> music;
        std::map<std::string, Mix_Chunk *> sounds;
    
        std::map<std::string, int> musicVolumes;
        std::map<std::string, int> soundVolumes;
    
        std::string currentMusic;
    
        Sound();
        static Sound *instance;
    public:
        ~Sound();
        static Sound *GetInstance();
    
        void AddMusic(const std::string &name, const std::string &path, int volume);
        void PlayMusic(const std::string &name, int loops = -1);
        void StopMusic();
        void PauseMusic();
        void ResumeMusic();

        void AddSound(const std::string &name, const std::string &path, int volume);
        void PlaySound(const std::string &name, int loops = 0);
        void StopSound();
        void PauseSound();
        void ResumeSound();
    
    };

#endif // SOUND_HPP 