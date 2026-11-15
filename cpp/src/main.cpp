// Fight for the Seat: an Indian election campaign game. All candidates, parties and places are fictional.
#include <cstdlib>
#include <cstring>
#include "Game.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

static seat::Game* gGame = nullptr;

static void tick() {
    gGame->frame();
    seat::ui::pendingClick = false;
}

#ifdef __EMSCRIPTEN__
// The canvas is scaled by CSS to fit the page, so map browser coordinates back to 1280x720 ourselves.
// Clicks are also latched: if the button goes down and up between two frames, raylib never sees it pressed.
static void trackMouse(const EmscriptenMouseEvent* e) {
    double w = 1280, h = 720;
    emscripten_get_element_css_size("#canvas", &w, &h);
    if (w <= 0 || h <= 0) return;
    seat::ui::webMouse = true;
    seat::ui::webMousePos = {(float)(e->targetX * 1280.0 / w), (float)(e->targetY * 720.0 / h)};
}
static bool onMouseMove(int, const EmscriptenMouseEvent* e, void*) {
    trackMouse(e);
    return false;
}
static bool onMouseDown(int, const EmscriptenMouseEvent* e, void*) {
    trackMouse(e);
    if (e->button == 0) seat::ui::pendingClick = true;
    return false;  // let raylib handle the event too
}
#endif

int main(int argc, char** argv) {
    if (argc > 1 && std::strcmp(argv[1], "--selftest") == 0) {
        SetTraceLogLevel(LOG_NONE);
        return seat::runSelfTest(argc > 2 ? std::atoi(argv[2]) : 200);
    }
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Fight for the Seat");
    SetExitKey(KEY_NULL);
    static seat::Game game;
    gGame = &game;
    game.init();
#ifdef __EMSCRIPTEN__
    emscripten_set_mousedown_callback("#canvas", nullptr, false, onMouseDown);
    emscripten_set_mousemove_callback("#canvas", nullptr, false, onMouseMove);
    // "#test" in the URL drives the loop with a timer instead of requestAnimationFrame,
    // so automated playtests keep running in a background tab.
    int timerLoop = EM_ASM_INT({ return location.hash === '#test' ? 1 : 0; });
    emscripten_set_main_loop(tick, timerLoop ? 30 : 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose() && !game.wantsQuit()) tick();
    game.shutdown();
    CloseWindow();
#endif
    return 0;
}
