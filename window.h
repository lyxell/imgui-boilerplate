#pragma once

#include <SDL.h>
#include <functional>
#include <queue>
#include <string>

namespace window {
extern std::queue<SDL_Keysym> keyboard_input;
extern std::string text_input;
bool is_exiting();
void start_frame();
void end_frame();
void init();
void destroy();
}; // namespace window
