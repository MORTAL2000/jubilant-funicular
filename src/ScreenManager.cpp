#ifdef NTA_USE_IMGUI
    #include <imgui/imgui.h>
    #include <imgui/imgui_impl_sdl_gl3.h>
#endif

#include "nta/ScreenManager.h"
#include "nta/SystemManager.h"
#include "nta/CallbackManager.h"
#include "nta/Logger.h"

namespace nta {
    ScreenManager::ScreenManager(crstring title, float maxFPS, int width, int height) {
        m_window = SystemManager::getWindow(title, width, height);
        m_limiter.setMaxFPS(maxFPS);
    }
    ScreenManager::~ScreenManager() {
        if (!m_screens.empty()) destroy();
    }
    Screen* ScreenManager::getCurrScreen() const {
        /// \todo write log error if m_currScreen is out of range
        return in_range<int>(m_currScreen, 0, m_screens.size()-1) ? m_screens[m_currScreen] : nullptr;
    }
    float ScreenManager::getFPS() const {
        return m_limiter.getFPS();
    }
    void ScreenManager::addScreen(Screen* newScreen, int escIndex, int xIndex, crstring title) {
        Logger::writeToLog("Adding screen " + to_string(m_screens.size()) + " to ScreenManager...");
        Logger::indent();
        m_currScreen = m_screens.empty() ? 0 : m_currScreen;
        newScreen->setManager(this, SetManagerKey());
        newScreen->setIndices(m_screens.size(), escIndex, xIndex, SetIndicesKey());
        newScreen->setWindow((title == "") ? m_window->getTitle() : title, SetWindowKey());
        newScreen->init();
        m_screens.push_back(newScreen);
        check_error();
        Logger::unindent();
        Logger::writeToLog("Added screen");
    }
    void ScreenManager::switchScreen(int newIndex) {
        if (in_range<int>(newIndex, 0, m_screens.size()-1)) {
            Logger::writeToLog("Switching from screen " + to_string(m_currScreen) +
                               " (\"" + getCurrScreen()->getName() + "\") to screen " + 
                               to_string(newIndex) + " (\"" + m_screens[newIndex]->getName() +
                               "\")...");
            Logger::indent();
            getCurrScreen()->offFocus();
            Logger::writeToLog("called offFocus");
            check_error();
            ScreenSwitchInfo info(getCurrScreen()->getSwitchData(), m_currScreen);
            m_currScreen = newIndex;
            getCurrScreen()->onFocus(info);
            Logger::writeToLog("called onFocus");
            check_error();
            Logger::unindent();
            Logger::writeToLog("Switched screen");
        } else if (newIndex == -1) {
            Logger::writeToLog("Exiting ScreenManager...");
            getCurrScreen()->offFocus(); // necessary?
            m_currScreen = -1;
        }
    }
    void ScreenManager::run(void* initFocusData) {
        Screen* currScreen = nullptr;
        if (m_currScreen != -1) {
            getCurrScreen()->onFocus(ScreenSwitchInfo(initFocusData));
        }
        while (m_currScreen != -1) {
            currScreen = getCurrScreen();
            // This while loop used to be neat
            while (currScreen->getState() == ScreenState::RUNNING) {
                m_limiter.begin();
                CallbackManager::increment_frame();
                currScreen->handleInput();
                currScreen->update();
                #ifdef NTA_USE_IMGUI
                    ImGui_ImplSdlGL3_NewFrame(m_window->getSDLWindow(GetSDLWindowKey()));
                #endif
                currScreen->render();
                ErrorManager::handle_errors();
                m_limiter.end();
            }
            switch(currScreen->getState()) {
            case ScreenState::SWITCH:       switchScreen(currScreen->getNextIndex()); break;
            case ScreenState::SWITCH_ESC:   switchScreen(currScreen->getEscIndex()); break;
            case ScreenState::SWITCH_X:     switchScreen(currScreen->getXIndex()); break;
            case ScreenState::NONE:         Logger::writeErrorToLog("state of screen " +
                                                                    to_string(m_currScreen) +
                                                                    " is NONE",
                                                                    IMPOSSIBLE_BEHAVIOR); break;
            default: break; // should never happen
            }
        }
        Logger::writeToLog("Exited ScreenManager");
    }
    void ScreenManager::destroy() {
        Logger::writeToLog("Destroying ScreenManager...");
        for (auto screen : m_screens) {
            delete screen;
        }
        m_screens.clear();
        Logger::writeToLog("Destroyed ScreenManager...");
    }
}
