#pragma once

#include <SDL.h>
#include <functional>
#include <queue>

namespace window {
    extern std::queue<SDL_Keycode> keyboard_input;
    bool is_exiting();
    void start_frame();
    void end_frame();
    void init();
    void destroy();
};
