#include "window.hpp"
#include "imgui.h"

int main() {
    window::init();
    bool show_demo_window = true;
    bool show_another_window = false;
    while (!window::is_exiting()) {
        window::start_frame();
        ImGui::ShowDemoWindow(&show_demo_window);
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");
            ImGui::Text("This is some useful text.");
            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::Checkbox("Another Window", &show_another_window);
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            if (ImGui::Button("Button")) {
                counter++;
            }
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
            ImGui::End();
        }
        if (show_another_window) {
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
        window::end_frame();
    }
    window::destroy();
    return 0;
}
