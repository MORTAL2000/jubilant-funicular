#include "ScreenManager.h"
#include "SystemManager.h"
#include "Logger.h"

#include <iostream>

namespace nta {
    ScreenManager::ScreenManager(crstring title, float maxFPS) {
        m_window = SystemManager::getWindow(title);
        m_limiter.setMaxFPS(maxFPS);
    }
    ScreenManager::~ScreenManager() {
    }
    Screen* ScreenManager::getCurrScreen() const {
        return inRange<int>(m_currScreen, 0, m_screens.size()-1) ? m_screens[m_currScreen] : nullptr;
    }
    float ScreenManager::getFPS() const {
        return m_limiter.getFPS();
    }
    void ScreenManager::addScreen(Screen* newScreen, int escIndex, int xIndex, crstring title) {
        Logger::writeToLog("Adding screen " + to_string(m_screens.size()) + " to screen manger...");
        m_currScreen = m_screens.empty() ? 0 : m_currScreen;
        newScreen->setManager(this, SetManagerKey());
        newScreen->setIndices(m_screens.size(), escIndex, xIndex);
        newScreen->setWindow(((title == "") ? m_window->getTitle() : title));
        newScreen->init();
        m_screens.push_back(newScreen);
        Logger::writeToLog("Added screen");
    }
    void ScreenManager::switchScreen(int newIndex) {
        if (inRange<int>(newIndex, 0, m_screens.size()-1)) {
            Logger::writeToLog("Switching to screen " + to_string(newIndex) +
                               " from screen " + to_string(m_currScreen) + "...");
            getCurrScreen()->offFocus();
            m_currScreen = newIndex;
            getCurrScreen()->onFocus();
            Logger::writeToLog("Switched screen");
        } else if (newIndex == -1) {
            Logger::writeToLog("Exiting ScreenManager...");
            getCurrScreen()->offFocus(); // necessary?
            m_currScreen = -1;
        }
    }
    void ScreenManager::run() {
        static Screen* currScreen = nullptr;
        if (m_currScreen != -1) getCurrScreen()->onFocus();
        while (m_currScreen != -1) {
            currScreen = getCurrScreen();
            while (currScreen->getState() == ScreenState::RUNNING) {
                m_limiter.begin();
                currScreen->handleInput();
                currScreen->update();
                currScreen->render();
                m_limiter.end();
            }
            switch(currScreen->getState()) {
            case ScreenState::SWITCH:       switchScreen(currScreen->getNextIndex()); break;
            case ScreenState::SWITCH_ESC:   switchScreen(currScreen->getEscIndex()); break;
            case ScreenState::SWITCH_X:     switchScreen(currScreen->getXIndex()); break;
            case ScreenState::NONE:         Logger::writeErrorToLog("state of screen " +
                                                                    to_string(m_currScreen) +
                                                                    " is NONE"); break;
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
