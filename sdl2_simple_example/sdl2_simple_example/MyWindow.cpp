#include "MyWindow.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL2/SDL_opengl.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "GameObject.h"
#include "Variables.h"
#include "Importer.h"
#include "Renderer.h"
#include "ConsoleWindow.h"
#include "ProjectWindow.h"
#include "InspectorWindow.h"
#include "HierarchyWindow.h"
#include "SceneWindow.h"

#include <IL/il.h>
#include <IL/ilu.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/sysctl.h>  // To obtain memory in macOS
#include <unistd.h>      // To obtain memory in Linux
#endif

extern Renderer renderer;
extern Importer importer;
extern std::vector<GameObject*> gameObjects;
extern MyWindow* window;

InspectorWindow inspectorWindow;
HierarchyWindow hierarchyWindow;
ConsoleWindow consoleWindow;
ProjectWindow projectWindow;
SceneWindow sceneWindow;

ImGuiIO* g_io = nullptr;

static bool showAboutWindow = false;
static bool showGithubWindow = false;
static bool showConfig = false;

static bool darkTheme = true;
static bool lightTheme = false;
static bool winterTheme = false;

void hideConsoleWindow() {
#if defined(_WIN32)
    HWND hwnd = GetConsoleWindow();
    if (hwnd != NULL) {
        ShowWindow(hwnd, SW_HIDE);
    }
#endif
}

// MyWindow builder initializing SDL, OpenGL and ImGui
MyWindow::MyWindow(const std::string& title, int w, int h) : _width(w), _height(h), selectedObject(nullptr) {
    hideConsoleWindow();

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    _window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!_window) throw std::exception(SDL_GetError());

    _ctx = SDL_GL_CreateContext(_window);
    if (!_ctx) throw std::exception(SDL_GetError());
    if (SDL_GL_MakeCurrent(_window, _ctx) != 0) throw std::exception(SDL_GetError());
    if (SDL_GL_SetSwapInterval(1) != 0) throw std::exception(SDL_GetError());

    ImGui::CreateContext();

    g_io = &ImGui::GetIO();
    g_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    g_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    g_io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui_ImplSDL2_InitForOpenGL(_window, _ctx);
    ImGui_ImplOpenGL3_Init("#version 130");

    configMyWindow();
}

// Destructor that frees memory and closes ImGui and SDL
MyWindow::~MyWindow() {
    for (GameObject* obj : gameObjects) {
        delete obj;
    }
    gameObjects.clear();


    SDL_GL_DeleteContext(_ctx);
    SDL_DestroyWindow(static_cast<SDL_Window*>(_window));
    ImGui_ImplSDL2_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
}

// ImGui colour and padding styles configuration
void MyWindow::configMyWindow()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowPadding = ImVec2(0.0f, 0.0f);

    ImVec4* colors = style.Colors;

    if (darkTheme) {
        colors[ImGuiCol_WindowBg] = ImColor(40, 40, 40);
        colors[ImGuiCol_MenuBarBg] = ImColor(219, 112, 147);
        colors[ImGuiCol_FrameBg] = ImColor(50, 50, 50);
        colors[ImGuiCol_DockingPreview] = ImColor(60, 90, 120);
        colors[ImGuiCol_Tab] = ImColor(183, 110, 121);
        colors[ImGuiCol_TabHovered] = ImColor(219, 112, 147);
        colors[ImGuiCol_TabActive] = ImColor(219, 112, 147);
        colors[ImGuiCol_TabUnfocused] = ImColor(183, 110, 121);
        colors[ImGuiCol_TabUnfocusedActive] = ImColor(70, 70, 80);
        colors[ImGuiCol_TitleBg] = ImColor(45, 45, 50);
        colors[ImGuiCol_TitleBgActive] = ImColor(60, 60, 70);
        colors[ImGuiCol_TitleBgCollapsed] = ImColor(45, 45, 50);
        colors[ImGuiCol_PopupBg] = ImColor(50, 50, 50);
        colors[ImGuiCol_ScrollbarBg] = ImColor(70, 70, 70);
        colors[ImGuiCol_ScrollbarGrab] = ImColor(181, 114, 129);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(219, 112, 147);
        colors[ImGuiCol_Border] = ImColor(219, 112, 147);
        colors[ImGuiCol_Button] = ImColor(183, 110, 121);
        colors[ImGuiCol_ButtonHovered] = ImColor(181, 114, 129);
        colors[ImGuiCol_ButtonActive] = ImColor(219, 112, 147);
        colors[ImGuiCol_Text] = ImColor(220, 220, 220);
        colors[ImGuiCol_Header] = ImColor(40, 40, 40);
        colors[ImGuiCol_HeaderActive] = ImColor(219, 112, 147);
        colors[ImGuiCol_HeaderHovered] = ImColor(181, 114, 129);
        darkTheme = true;
        lightTheme = false;
        winterTheme = false;
    }
    if (lightTheme) {
        colors[ImGuiCol_WindowBg] = ImColor(255, 240, 230);
        colors[ImGuiCol_MenuBarBg] = ImColor(255, 112, 150);
        colors[ImGuiCol_FrameBg] = ImColor(247, 202, 208);
        colors[ImGuiCol_DockingPreview] = ImColor(102, 155, 188);
        colors[ImGuiCol_Tab] = ImColor(248, 249, 250);
        colors[ImGuiCol_TabHovered] = ImColor(233, 236, 239);
        colors[ImGuiCol_TabActive] = ImColor(222, 226, 230);
        colors[ImGuiCol_TabUnfocused] = ImColor(206, 212, 218);
        colors[ImGuiCol_TabUnfocusedActive] = ImColor(173, 181, 189);
        colors[ImGuiCol_TitleBg] = ImColor(249, 190, 199);
        colors[ImGuiCol_TitleBgActive] = ImColor(255, 153, 172);
        colors[ImGuiCol_TitleBgCollapsed] = ImColor(249, 190, 199);
        colors[ImGuiCol_PopupBg] = ImColor(250, 224, 228);
        colors[ImGuiCol_ScrollbarBg] = ImColor(247, 202, 208);
        colors[ImGuiCol_ScrollbarGrab] = ImColor(255, 153, 172);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(255, 133, 161);
        colors[ImGuiCol_Border] = ImColor(255, 112, 150);
        colors[ImGuiCol_Button] = ImColor(251, 177, 189);
        colors[ImGuiCol_ButtonHovered] = ImColor(255, 133, 161);
        colors[ImGuiCol_ButtonActive] = ImColor(255, 112, 150);
        colors[ImGuiCol_Text] = ImColor(0, 0, 0);
        colors[ImGuiCol_Header] = ImColor(250, 224, 228);
        colors[ImGuiCol_HeaderActive] = ImColor(255, 153, 172);
        colors[ImGuiCol_HeaderHovered] = ImColor(255, 133, 161);
        lightTheme = true;
        darkTheme = false;
        winterTheme = false;
    }
    if (winterTheme) {
        colors[ImGuiCol_WindowBg] = ImColor(15, 15, 20);
        colors[ImGuiCol_MenuBarBg] = ImColor(40, 45, 60);
        colors[ImGuiCol_FrameBg] = ImColor(35, 40, 55);
        colors[ImGuiCol_DockingPreview] = ImColor(65, 80, 110);
        colors[ImGuiCol_Tab] = ImColor(50, 55, 70);
        colors[ImGuiCol_TabHovered] = ImColor(75, 90, 120);
        colors[ImGuiCol_TabActive] = ImColor(95, 115, 150);
        colors[ImGuiCol_TabUnfocused] = ImColor(40, 45, 60);
        colors[ImGuiCol_TabUnfocusedActive] = ImColor(65, 80, 110);
        colors[ImGuiCol_TitleBg] = ImColor(50, 55, 70);
        colors[ImGuiCol_TitleBgActive] = ImColor(95, 115, 150);
        colors[ImGuiCol_TitleBgCollapsed] = ImColor(40, 45, 60);
        colors[ImGuiCol_PopupBg] = ImColor(30, 35, 45);
        colors[ImGuiCol_ScrollbarBg] = ImColor(35, 40, 55);
        colors[ImGuiCol_ScrollbarGrab] = ImColor(85, 100, 130);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(105, 120, 150);
        colors[ImGuiCol_Border] = ImColor(95, 115, 150);
        colors[ImGuiCol_Button] = ImColor(135, 180, 230);
        colors[ImGuiCol_ButtonHovered] = ImColor(150, 190, 240);
        colors[ImGuiCol_ButtonActive] = ImColor(170, 210, 255);
        colors[ImGuiCol_Text] = ImColor(235, 235, 235);
        colors[ImGuiCol_Header] = ImColor(50, 55, 70);
        colors[ImGuiCol_HeaderActive] = ImColor(95, 115, 150);
        colors[ImGuiCol_HeaderHovered] = ImColor(75, 90, 120);
        colors[ImGuiCol_CheckMark] = ImColor(135, 180, 230);
        colors[ImGuiCol_SliderGrab] = ImColor(85, 100, 130);
        colors[ImGuiCol_SliderGrabActive] = ImColor(95, 115, 150);
        colors[ImGuiCol_Separator] = ImColor(60, 70, 90);
        colors[ImGuiCol_SeparatorHovered] = ImColor(75, 90, 120);
        colors[ImGuiCol_SeparatorActive] = ImColor(95, 115, 150);
        colors[ImGuiCol_DragDropTarget] = ImColor(65, 80, 110);

        lightTheme = false;
        darkTheme = false;
        winterTheme = true;
    }
}

void MyWindow::createDockSpace() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags dockspace_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("MainDockSpace", nullptr, dockspace_flags);
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    createMainMenu();
    hierarchyWindow.render(gameObjects, selectedObjects, selectedObject);
    inspectorWindow.render(selectedObject);
    projectWindow.render();
    sceneWindow.render();
    console.displayConsole();

    ImGui::End();
}

// Selects an object in the scene
void MyWindow::selectObject(GameObject* obj) {
    const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);

    if (keyboardState[SDL_SCANCODE_LCTRL] || keyboardState[SDL_SCANCODE_RCTRL]) {
        auto it = std::find(selectedObjects.begin(), selectedObjects.end(), obj);
        if (it != selectedObjects.end()) {
            selectedObjects.erase(it);
        }
        else {
            selectedObjects.push_back(obj);
        }
    }
    else {
        selectedObjects.clear();
        selectedObjects.push_back(obj);
    }

    console.addLog("Selected objects:");
    for (GameObject* o : selectedObjects) {
        console.addLog(("  - " + o->getName()).c_str());
    }
}


void MyWindow::createMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save Scene")) {
                /* std::string filePath = "scene.json";
                 std::vector<MeshData> meshes;

                 for (GameObject* obj : gameObjects) {
                     meshes.push_back(obj->toMeshData());
                 }

                 importer.saveScene(filePath, meshes); */
            }
            if (ImGui::MenuItem("Load Scene")) {
                //  std::string filePath = "scene.json";  
                //  std::vector<MeshData> meshes = importer.loadScene(filePath);  

                //  gameObjects.clear();
                //  for (const MeshData& mesh : meshes) {
                //      GameObject* obj = new GameObject(mesh.name, mesh, 0);
                //      gameObjects.push_back(obj);
                //  }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("GameObject")) {
            if (ImGui::MenuItem("Empty Object")) { GameObject::createEmptyObject("Empty", gameObjects); }
            if (ImGui::MenuItem("Sphere")) { GameObject::createPrimitive("Sphere", gameObjects); }
            if (ImGui::MenuItem("Cube")) { GameObject::createPrimitive("Cube", gameObjects); }
            if (ImGui::MenuItem("Cylinder")) { GameObject::createPrimitive("Cylinder", gameObjects); }
            if (ImGui::MenuItem("Cone")) { GameObject::createPrimitive("Cone", gameObjects); }
            if (ImGui::MenuItem("Torus")) { GameObject::createPrimitive("Torus", gameObjects); }
            if (ImGui::MenuItem("Plane")) { GameObject::createPrimitive("Plane", gameObjects); }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Configuration")) {
            if (ImGui::MenuItem("Show Config")) {
                showConfig = true;
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("GitHub")) {
                showGithubWindow = true;
            }
            if (ImGui::MenuItem("About")) {
                showAboutWindow = true;
            }
            if (ImGui::MenuItem("Exit")) {
                SDL_Event quit_event;
                quit_event.type = SDL_QUIT;
                SDL_PushEvent(&quit_event);
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();

        if (showConfig) {
            ImGui::Begin("Config", nullptr);

            if (ImGui::CollapsingHeader("Window Theme")) {
                ImGui::Text("Choose a mode");
                if (ImGui::Checkbox("Dark theme", &darkTheme)) {
                    lightTheme = false;
                    winterTheme = false;
                    configMyWindow();
                }

                if (ImGui::Checkbox("Light theme", &lightTheme)) {
                    darkTheme = false;
                    winterTheme = false;
                    configMyWindow();
                }

                if (ImGui::Checkbox("Winter theme", &winterTheme)) {
                    lightTheme = false;
                    darkTheme = false;
                    configMyWindow();
                }

            }

            ImGui::Separator();

            if (ImGui::CollapsingHeader("FPS Graph")) {
                ImGui::Text("FPS: %d", _fps);
                ImGui::PlotLines("FPS", _fpsHistory.data(), _fpsHistory.size(), 0, nullptr, 0.0f, 100.0f, ImVec2(0, 80));

            }

            ImGui::Separator();

            if (ImGui::CollapsingHeader("Window Settings")) {
                ImGui::InputInt("Width", &variables->windowWidth);
                ImGui::InputInt("Height", &variables->windowHeight);
                if (ImGui::Checkbox("Fullscreen", &variables->fullscreen)) {}

                if (ImGui::Button("Apply Changes")) {
                    SDL_SetWindowSize(_window, variables->windowWidth, variables->windowHeight);
                    if (variables->fullscreen) {
                        SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    }
                    else {
                        SDL_SetWindowFullscreen(_window, 0);
                    }
                }
            }

            ImGui::Separator();

            /*if (ImGui::CollapsingHeader("Texture Settings")) {
                ImGui::SliderFloat("Texture Filter Quality", &variables->textureFilterQuality, 0.0f, 2.0f);
                ImGui::SliderFloat("Anisotropic Filter", &variables->textureAnisotropicLevel, 1.0f, 16.0f);
            }*/
            if (ImGui::CollapsingHeader("Object Settings")) {
                GameObject* selectedObject = variables->window->selectedObject;
                if (selectedObject) {
                    char nameBuffer[256];
                    strncpy_s(nameBuffer, selectedObject->name.c_str(), sizeof(nameBuffer) - 1);

                    if (ImGui::InputText("Object Name", nameBuffer, sizeof(nameBuffer))) {
                        selectedObject->name = std::string(nameBuffer);
                    }
                }
                else {
                    ImGui::Text("No object selected");
                }
            }

            ImGui::Separator();

            if (ImGui::CollapsingHeader("Info")) {

                MEMORYSTATUSEX statex;
                statex.dwLength = sizeof(statex);
                GlobalMemoryStatusEx(&statex);

                ImGui::Text("Memory Usage:");
                ImGui::Text("Total RAM: %.2f GB", statex.ullTotalPhys / (1024.0 * 1024.0 * 1024.0));
                ImGui::Text("Available RAM: %.2f GB", statex.ullAvailPhys / (1024.0 * 1024.0 * 1024.0));
                ImGui::Text("Used RAM: %.2f GB", (statex.ullTotalPhys - statex.ullAvailPhys) / (1024.0 * 1024.0 * 1024.0));

                ImGui::Separator();

                SDL_version sdl_version;
                SDL_GetVersion(&sdl_version);
                ImGui::Text("SDL Version: %d.%d.%d", sdl_version.major, sdl_version.minor, sdl_version.patch);
                ImGui::Separator();

                const char* glVersion = (const char*)glGetString(GL_VERSION);
                const char* glRenderer = (const char*)glGetString(GL_RENDERER);
                const char* glVendor = (const char*)glGetString(GL_VENDOR);
                ImGui::Text("OpenGL Version: %s", glVersion);
                ImGui::Text("OpenGL Renderer: %s", glRenderer);
                ImGui::Text("OpenGL Vendor: %s", glVendor);
                ImGui::Separator();

                int devilVersion = ilGetInteger(IL_VERSION_NUM);
                ImGui::Text("DevIL Version: %d.%d.%d", (devilVersion / 100), (devilVersion / 10) % 10, devilVersion % 10);

            }

            ImGui::Separator();

            if (ImGui::Button("Close")) {
                showConfig = false;
            }

            ImGui::End();
        }

        if (showAboutWindow)
        {
            ImGui::Begin("About", &showAboutWindow);
            ImGui::Text("PuchiBeruta is a simple game engine for learning purposes");
            ImGui::Text("Version: 1.0.0");
            ImGui::Text("Developed by videogame design & development UPC students");
            ImGui::Text("-------------------------");
            ImGui::Text("Team: ");

            if (ImGui::Button("Maria Perarnau")) {
                system("start https://github.com/MariaPerarnau");
            }
            if (ImGui::Button("Rebeca Fernandez")) {
                system("start https://github.com/Becca203");
            }
            if (ImGui::Button("Carla Puga")) {
                system("start https://github.com/cpugaper");
            }

            ImGui::Text("-------------------------");
            ImGui::Text("");
            if (ImGui::Button("Close")) {
                showAboutWindow = false;
            }
            ImGui::End();
        }

        if (showGithubWindow) {
            ImGui::Begin("GitHub", &showGithubWindow);
            ImGui::Text("Visit our GitHub page:");
            if (ImGui::Button("Open GitHub")) {
                system("start https://github.com/cpugaper/puchiBeruta");
            }
            if (ImGui::Button("Close")) {
                showGithubWindow = false;
            }
            ImGui::End();
        }
    }
}

// Updated interface and rendering of ImGui content
void MyWindow::swapBuffers() {
    _currentTime = SDL_GetTicks();
    _frameCount++;

    if (_currentTime - _lastTime >= 1000) {
        _fps = _frameCount;
        _frameCount = 0;
        _lastTime = _currentTime;

        if (_fpsHistory.size() >= 100) {
            _fpsHistory.erase(_fpsHistory.begin());
        }
        _fpsHistory.push_back((float)_fps);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    createDockSpace();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (g_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(_window, _ctx);
    }

    SDL_GL_SwapWindow(static_cast<SDL_Window*>(_window));
}