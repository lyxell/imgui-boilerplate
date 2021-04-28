imgui-boilerplate is a thin layer on top of imgui
to aid bootstrapping.

### usage

```c++
#include "imgui-boilerplate/window.h"
#include "imgui.h"

int main() {
    window::init();
    while (!window::is_exiting()) {
        window::start_frame();
        ImGui::ShowDemoWindow();
        window::end_frame();
    }
    window::destroy();
    return 0;
}
```
