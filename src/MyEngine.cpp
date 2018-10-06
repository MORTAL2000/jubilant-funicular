#ifdef NTA_USE_DEVIL
    #include <IL/il.h>
    #include <IL/ilu.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#ifdef NTA_USE_IMGUI
    #include <imgui/imgui.h>
    #include <imgui/imgui_impl_sdl_gl3.h>
#endif

#include "nta/MyEngine.h"
#include "nta/ResourceManager.h"
#include "nta/WindowManager.h"
#include "nta/CallbackManager.h"
#include "nta/Logger.h"
#include "nta/Random.h"
#include "nta/utils.h"

#ifdef NTA_USE_AUDIO
    #include "nta/AudioManager.h"
#endif

namespace nta {
    void init(int gl_major_version, int gl_minor_version, bool use_gl_core) {
        Logger::createLog();
        Logger::writeToLog("Initializing Engine...");
        Logger::indent();
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            Logger::writeErrorToLog("Failed to initialize SDL: " + utils::to_string(SDL_GetError()),
                                    SDL_FAILURE);
        }
        if (gl_major_version > 0) SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major_version);
        if (gl_minor_version >= 0) SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor_version);
        if (use_gl_core) SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR));
        #ifdef NTA_USE_IMGUI
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui::StyleColorsDark();
        #endif
        #ifdef NTA_USE_DEVIL
            ilInit();
            iluInit();
        #endif
        Random::init();
        #ifdef NTA_USE_AUDIO
            AudioManager::init();
        #endif
        CallbackManager::init();
        Logger::unindent();
        Logger::writeToLog("Initialized Engine");
    }
    void cleanup() {
        #ifdef NTA_USE_IMGUI
            ImGui_ImplSdlGL3_Shutdown();
            ImGui::DestroyContext();
        #endif
        ResourceManager::destroy();
        WindowManager::destroy();
        #ifdef NTA_USE_AUDIO
            AudioManager::destroy();
        #endif
        CallbackManager::destroy();
        if (TTF_WasInit()) TTF_Quit();
        SDL_Quit();
    }
    bool check_error() {
        std::string gl_err;
        int err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            gl_err += gl_err == "" ? "" : "\n";
            gl_err += utils::to_string(gluErrorString(err)) + " (" + utils::to_string(err) + ")";
        }
        if (gl_err != "") {
            Logger::writeToLog("GL Error(s): " + gl_err);
        }
        
        std::string sdl_err = SDL_GetError();
        if (sdl_err != "") {
            Logger::writeToLog("SDL Error: "+sdl_err);
            SDL_ClearError();
        }
        return err != GL_NO_ERROR || sdl_err != "";
    }
}
