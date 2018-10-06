/*
 * Here's an example of a game with multiple windows. The two windows share a game
 * state. In one window, the player moves left and right dodging falling blocks that
 * can only be seen in the other window.
 */
#include <iostream>
#include <thread>

#include <nta/ScreenManager.h>
#include <nta/InputManager.h>
#include <nta/SpriteBatch.h>
#include <nta/GLSLProgram.h>
#include <nta/Camera2D.h>
#include <nta/Logger.h>

using namespace nta;
using namespace std;
using namespace glm;

class SquareScreen : public Screen {
private:
    // Stores the GLSL code used during rendering
    // the code will be loaded from an external file
    GLSLProgram* m_simple_prog = nullptr;
    // A "batch" (collection) of "sprites" (possibly textured rectangles)
    // Rendering happens by first collecting all sprites in a batch, and then
    //   drawing the batch all in one go
    SpriteBatch m_batch;
    // Defines the camera/coordinate system used when rendering
    // by default the center of the screen is (0,0)
    //            the top right is            (100,100)
    //            the bottom left is          (-100,-100)
    Camera2D m_camera;

    vec2 top_left;
public:
    SquareScreen() : top_left(-25, 25) {}
    ~SquareScreen() {}
    // The following three functions must be defined children of the Screen class
    void init();
    void update();
    void render();
};

void SquareScreen::init() {
    Logger::writeToLog("Initializing screen...");

    // Admitedlly, the simple2D I'm using is more complicated than need be
    // Loads a GLSLProgram (vertex and fragment shader) from a pair of files
    //   named "simple2D.vert" and "simple2D.frag"
    m_simple_prog = m_manager->getGLSLProgram("simple2D");
    if (!m_simple_prog->isLinked()) {
        // Adds the various attributes used by simple2D.vert
        // the order here must match the order of the fields of Vertex2D
        m_simple_prog->addAttribute("pos");
        m_simple_prog->addAttribute("color");
        m_simple_prog->addAttribute("uv");
        m_simple_prog->addAttribute("hasTexture");
        m_simple_prog->linkShaders();
    }

    m_batch.init();

    Logger::writeToLog("screen initialized");
}

void SquareScreen::update() {
    /*
    // If the enter key was just pressed...
    if (InputManager::justPressed(SDLK_RETURN)) {
        // Let the ScreenManager know that it is time to switch screens
        // Switch to the screen at index -1 (i.e. exit the ScreenManager)
        m_state = ScreenState::SWITCH;
        m_nextIndex = -1;
    }
    */
    if (getInput().isPressed(SDLK_w)) {
        top_left.y += 1;
    }
    if (getInput().isPressed(SDLK_a)) {
        top_left.x -= 1;
    }
    if (getInput().isPressed(SDLK_s)) {
        top_left.y -= 1;
    }
    if (getInput().isPressed(SDLK_d)) {
        top_left.x += 1;
    }
    /*
    cout<<lock_stream<<"The active mouse window is "<<Window::getMouseFocus()<<endl
                     <<"           key   window is "<<Window::getKeyboardFocus()<<endl
        <<unlock_stream;
    */
    while (abs(top_left.x) > 100) top_left.x -= sign(top_left.x)*200;
    while (abs(top_left.y) > 100) top_left.y -= sign(top_left.y)*200;
}

void SquareScreen::render() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Collect all the glyphs to be drawn
    m_batch.begin(); { // Creating a new scope like this isn't necessary (just a personal preference)
        // Draws a square whose topleft corner is at (-25, 25) and whose
        // width and height are both 50. The uv (i.e. texture) coordinates
        // used are given by a square whose topleft corner is at (0, 0) and
        // whose width and height are both 0. This is an empty square which
        // would be problematic except that the texture id suuplied here is
        // 0, so this glyph won't actually use a texture at all (i.e. since
        // the third parameter is 0, the second one is irrelevant). Finally,
        // this square is colored red: (1,0,0,1) in RGBA.
        m_batch.addGlyph(vec4(top_left, 50, 50), vec4(0), 0, vec4(1,0,0,1));
    } m_batch.end();

    // Actually draw stuff
    m_simple_prog->use(); {
        glUniformMatrix3fv(m_simple_prog->getUniformLocation("camera"), 1, GL_FALSE,
                           &m_camera.getCameraMatrix()[0][0]);
        m_batch.render();
    } m_simple_prog->unuse();

    m_window->swapBuffers();
}

int main(int argc, char* argv[]) {
    // Remember to initialize the library
    nta::init();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    thread t1 = thread([]{
        ScreenManager s1("Player Screen", 60);
        s1.addScreen(new SquareScreen);
        s1.run();
        s1.destroy();
    });
    
    thread t2 = thread([]{
        ScreenManager s2("Block Screen", 60);
        s2.addScreen(new SquareScreen);
        s2.run();
        s2.destroy();
    });
    
    t1.join();
    t2.join();

    cleanup();
    Logger::writeToLog("Program exited cleanly");

    return 0;
}
