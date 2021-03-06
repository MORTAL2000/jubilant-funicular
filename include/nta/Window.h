#ifndef NTA_WINDOW_H_INCLUDED
#define NTA_WINDOW_H_INCLUDED

#include <mutex>

#include <SDL2/SDL.h>

#include "nta/MyEngine.h"
#include "nta/Wrapper.h"

namespace nta {
    class WindowManager;
    typedef utils::Wrapper<uint32_t, struct WIDTAG> WindowID;
    /// Flags used for creating a window
    enum WindowFlags {
        INVISIBLE = 0x1, 
        FULLSCREEN = 0x2, 
        BORDERLESS = 0x4, 
        NOTRESIZEABLE = 0x8,
        HIGHDPI = 0x10
    };
    /// Key for unlocking the GetSDLWindow() "private" function of class Window
    class GetSDLWindowKey {
        friend class ScreenManager;
        GetSDLWindowKey() {}
        GetSDLWindowKey(const GetSDLWindowKey&);
        GetSDLWindowKey& operator=(const GetSDLWindowKey&);
    };
    /// Represent a window
    class Window {
    private:
        static WindowID m_keyboard_focus;
        static std::mutex m_keyboard_mutex;
        static WindowID m_mouse_focus;
        static std::mutex m_mouse_mutex;

        /// creates a window
        void createWindow(crstring title, int width, int height, int flags = 0);
        /// the window
        SDL_Window* m_window;
        /// the title of the window
        std::string m_title;
        /// the dimensions of the window
        int m_width, m_height;
    public:
        /// constructor and destructor
        Window();
        ~Window();
        /// returns the underlying window
        SDL_Window* getSDLWindow(GetSDLWindowKey key) const;
        /// returns the window's dimensions
        glm::vec2 getDimensions() const;
        /// returns the window's title
        std::string getTitle() const;
        /// return the window's id
        WindowID getID() const { return SDL_GetWindowID(m_window); }
        /// returns the width of the window
        int getWidth() const;
        /// returns the height of the window
        int getHeight() const;
        bool hasKeyboardFocus() const { return getKeyboardFocus() == getID(); }
        bool hasMouseFocus() const { return getMouseFocus() == getID(); }
        /// resizes the window
        void resize(int width, int height);
        /// updates the window's stored dimensions
        void setDimensions(int width, int height);
        /// updates the screen
        void swapBuffers() const;
        
        static WindowID getKeyboardFocus();
        static WindowID getMouseFocus();
        static void setKeyboardFocus(const WindowID& win);
        static void setMouseFocus(const WindowID& win);

        friend class WindowManager;
    };
}

#endif // NTA_WINDOW_H_INCLUDED
