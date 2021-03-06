#include "window.h"

// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows,
// inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no
// standard header to access modern OpenGL functions easily. Alternatives are
// GLEW, Glad, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder +
// read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "fonts/FiraMono_Regular.h"
#include "fonts/FiraMono_Medium.h"
#include "fonts/FiraSans_Regular.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include <SDL.h>
#include <stdio.h>

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load
//  OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a
//  few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.),
//  or chose to manually implement your own.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
// Initialize with gl3wInit()
#include <GL/gl3w.h>
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
// Initialize with glewInit()
#include <GL/glew.h>
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
// Initialize with gladLoadGL()
#include <glad/glad.h>
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
// Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#include <glad/gl.h>
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
// GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#define GLFW_INCLUDE_NONE
// Initialize with glbinding::Binding::initialize()
#include <glbinding/Binding.h>
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
// GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#define GLFW_INCLUDE_NONE
// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// state
static bool is_exiting_value = false;
static ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
static SDL_Window* window_ptr;
static SDL_GLContext gl_context;

namespace window {

static ImFont* heading_font;
static ImFont* normal_font;

std::queue<SDL_Keysym> keyboard_input;
std::string text_input;
std::vector<ImFont*> fonts;

void heading(const char* str) {
    ImGui::PushFont(heading_font);
    ImGui::Text(str);
    ImGui::PopFont();
}

void text(const char* str) {
    ImGui::PushFont(normal_font);
    ImGui::Text(str);
    ImGui::PopFont();
}

void init() {

    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have
    // performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or
    // disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) !=
        0) {
        printf("Error: %s\n", SDL_GetError());
        exit(-1);
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window_ptr = SDL_CreateWindow(
        "Application", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    gl_context = SDL_GL_CreateContext(window_ptr);
    SDL_GL_MakeCurrent(window_ptr, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    // glad2 recommend using the windowing library loader instead of the
    // (optionally) bundled one.
    bool err = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) {
        return (glbinding::ProcAddress)SDL_GL_GetProcAddress(name);
    });
#else
    // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to
    // requires some form of initialization.
    bool err = false;
#endif
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        exit(1);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // Enable Gamepad Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // Enable Multi-Viewport / Platform Windows
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so
    // platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window_ptr, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font.
    //   You can also load multiple fonts and use
    //   ImGui::PushFont()/PopFont() to select them.
    //
    // - AddFontFromFileTTF() will return the ImFont* so you can store
    //   it if you need to select the font among multiple.
    //
    // - If the file cannot be loaded, the function will return NULL.
    //   Please handle those errors in your application (e.g. use an
    //   assertion, or display an error and quit).
    //
    // - The fonts will be rasterized at a given size (w/ oversampling)
    //   and stored into a texture when calling
    //   ImFontAtlas::Build()/GetTexDataAsXXXX(), which
    //   ImGui_ImplXXXX_NewFrame below will call.
    //
    // - Read 'docs/FONTS.md' for more instructions and details.
    //
    // - Remember that in C/C++ if you want to include a backslash
    // \ in a string literal you need to write a double backslash \\ !
    //
    // io.Fonts->AddFontDefault();
    //
    // io.Fonts->AddFontFromFileTTF(
    //     "../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //
    // io.Fonts->AddFontFromFileTTF(
    //  "../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //
    // io.Fonts->AddFontFromFileTTF(
    //  "../../misc/fonts/DroidSans.ttf", 16.0f);
    //
    // io.Fonts->AddFontFromFileTTF(
    //  "../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF(
    //  "c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
    //  NULL, io.Fonts->GetGlyphRangesJapanese());
    //
    // IM_ASSERT(font != NULL);
    
    // default font
    io.Fonts->AddFontFromMemoryCompressedTTF(FiraMono_Regular_compressed_data,
                                             FiraMono_Regular_compressed_size, 16.0f);

    heading_font = io.Fonts->AddFontFromMemoryCompressedTTF(FiraSans_Regular_compressed_data,
                                             FiraSans_Regular_compressed_size, 22.0f);

    normal_font = io.Fonts->AddFontFromMemoryCompressedTTF(FiraSans_Regular_compressed_data,
                                             FiraSans_Regular_compressed_size, 18.0f);

    // Style
    style.WindowMenuButtonPosition = -1;
    style.TabRounding = 0.0f;
    style.WindowRounding = 8.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.FramePadding = ImVec2(12.0f, 12.0f);
    style.IndentSpacing = 12.0f;
    ImVec4* c = style.Colors;

    c[ImGuiCol_BorderShadow] = ImVec4(0.75f, 0.75f, 0.75f, 0.00f);
    c[ImGuiCol_Border] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
    c[ImGuiCol_Button] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    c[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.92f, 0.00f, 1.00f);
    c[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
    c[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c[ImGuiCol_DockingPreview] = ImVec4(1.00f, 0.91f, 0.00f, 1.00f);
    c[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.91f, 0.00f, 1.00f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.79f, 0.79f, 0.79f, 0.67f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);
    c[ImGuiCol_FrameBg] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
    c[ImGuiCol_HeaderActive] = ImVec4(1.00f, 0.91f, 0.00f, 1.00f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    c[ImGuiCol_Header] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    c[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.28f);
    c[ImGuiCol_NavHighlight] = ImVec4(0.79f, 0.79f, 0.79f, 1.00f);
    c[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    c[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    c[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    c[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    c[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    c[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    c[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
    c[ImGuiCol_ResizeGripActive] = ImVec4(0.80f, 0.80f, 0.80f, 0.95f);
    c[ImGuiCol_ResizeGripHovered] = ImVec4(0.79f, 0.79f, 0.79f, 0.67f);
    c[ImGuiCol_ResizeGrip] = ImVec4(0.78f, 0.78f, 0.78f, 0.20f);
    c[ImGuiCol_ScrollbarBg] = ImVec4(0.78f, 0.78f, 0.78f, 0.53f);
    c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.71f, 0.71f, 0.71f, 1.00f);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    c[ImGuiCol_SeparatorActive] = ImVec4(1.00f, 0.91f, 0.00f, 1.00f);
    c[ImGuiCol_SeparatorHovered] = ImVec4(0.63f, 0.63f, 0.63f, 0.78f);
    c[ImGuiCol_Separator] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    c[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.91f, 0.00f, 1.00f);
    c[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.91f, 0.00f, 1.00f);
    c[ImGuiCol_TabActive] = ImVec4(1.00f, 0.91f, 0.00f, 1.00f);
    c[ImGuiCol_TabHovered] = ImVec4(1.00f, 0.91f, 0.00f, 1.00f);
    c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.81f, 0.81f, 0.81f, 1.00f);
    c[ImGuiCol_TabUnfocused] = ImVec4(0.93f, 0.93f, 0.93f, 1.00f);
    c[ImGuiCol_Tab] = ImVec4(0.93f, 0.93f, 0.93f, 1.00f);
    c[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    c[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    c[ImGuiCol_TableHeaderBg] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    c[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    c[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    c[ImGuiCol_TextDisabled] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
    c[ImGuiCol_TextSelectedBg] = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
    c[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    c[ImGuiCol_TitleBgActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    c[ImGuiCol_TitleBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

    //
    //
    SDL_StartTextInput();
}

bool is_exiting() { return is_exiting_value; }

void start_frame() {

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
    // tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data
    //      to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
    //      data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them
    // from your application based on those two flags.
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            is_exiting_value = true;
        }
        if (event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_WINDOWEVENT_CLOSE &&
            event.window.windowID == SDL_GetWindowID(window_ptr)) {
            is_exiting_value = true;
        }
        if (event.type == SDL_KEYDOWN) {
            keyboard_input.push(event.key.keysym);
        }
        if (event.type == SDL_TEXTINPUT) {
            text_input += event.text.text;
        }
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window_ptr);
    ImGui::NewFrame();
}

void end_frame() {
    keyboard_input = {};
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we
    // save/restore it to make it easier to paste this code elsewhere.
    // For this specific demo app we could also call
    // SDL_GL_MakeCurrent(window, gl_context) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }
    SDL_GL_SwapWindow(window_ptr);
}

void destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window_ptr);
    SDL_Quit();
}

} // namespace window
