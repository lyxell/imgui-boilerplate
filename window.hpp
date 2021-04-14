#include <SDL.h>
#include <functional>

namespace window {
    bool is_exiting();
    void start_frame();
    void end_frame();
    void init();
    void destroy();
};
