#include "nta/AudioManager.h"
#include "nta/Logger.h"
#include "nta/utils.h"

namespace nta {
    std::map<std::string, SoundEffect*>     AudioManager::m_effectMap;
    std::map<std::string, Music*>           AudioManager::m_musicMap;
    void AudioManager::init() {
        Logger::writeToLog("Initializing AudioManager...");
        if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) == -1) {
            Logger::writeErrorToLog("Failed to initialize SDL_mixer: "
                                    + utils::to_string(Mix_GetError()));
        }
        if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1) {
            Logger::writeErrorToLog("Failed to open audio: " + utils::to_string(Mix_GetError()));
        }
        Logger::writeToLog("Initialized AudioManager");
    }
    void AudioManager::destroy() {
        Logger::writeToLog("Destroying AudioManager...");
        for (auto& it : m_effectMap) {
            Logger::writeToLog("Deleting sound effect: " + it.first + "...");
            delete it.second;
            Logger::writeToLog("Deleted sound effect");
        }
        for (auto& it : m_musicMap) {
            Logger::writeToLog("Deleting music: " + it.first + "...");
            delete it.second;
            Logger::writeToLog("Deleted music");
        }
        m_effectMap.clear();
        m_musicMap.clear();
        Mix_CloseAudio();
        Mix_Quit();
        Logger::writeToLog("Destroyed AudioManager");
    }
    SoundEffect* AudioManager::getSoundEffect(crstring effectPath) {
        if (m_effectMap.find(effectPath) == m_effectMap.end()) {
            nta::Logger::writeToLog("Loading sound effect: " + effectPath + "...");
            Mix_Chunk* newChunk = Mix_LoadWAV((effectPath).c_str());
            if (!newChunk) {
                nta::Logger::writeErrorToLog("Unable to load sound effect: "
                                             + utils::to_string(Mix_GetError()));
            }
            m_effectMap[effectPath] = new SoundEffect(newChunk);
            nta::Logger::writeToLog("Loaded sound effect");
        }
        return m_effectMap[effectPath];
    }
    Music* AudioManager::getMusic(crstring musicPath) {
        if (m_musicMap.find(musicPath) == m_musicMap.end()) {
            nta::Logger::writeToLog("Loading music file: " + musicPath + "...");
            Mix_Music* newMusic = Mix_LoadMUS((musicPath).c_str());
            if (!newMusic) {
                nta::Logger::writeErrorToLog("Unable to load music file: "
                                             + utils::to_string(Mix_GetError()));
            }
            m_musicMap[musicPath] = new Music(newMusic);
            nta::Logger::writeToLog("Loaded music file");
        }
        return m_musicMap[musicPath];
    }
};
