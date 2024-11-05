#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include "MyWindow.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "GameObject.h"
#include <SDL2/SDL_opengl.h>
#include <vector>
#include <iostream>
extern Importer importer;

extern std::vector<GameObject*> gameObjects; 
extern MyWindow* window;

MyWindow::MyWindow(const std::string& title, int w, int h) : _width(w), _height(h), selectedObject(nullptr) {

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    _window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL);
    if (!_window) throw std::exception(SDL_GetError());

    _ctx = SDL_GL_CreateContext(_window);
    if (!_ctx) throw std::exception(SDL_GetError());
    if (SDL_GL_MakeCurrent(_window, _ctx) != 0) throw std::exception(SDL_GetError());
    if (SDL_GL_SetSwapInterval(1) != 0) throw std::exception(SDL_GetError());

    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(_window, _ctx);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGuiStyle& style = ImGui::GetStyle();

    // Colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImColor(250, 224, 228);
    colors[ImGuiCol_MenuBarBg] = ImColor(255, 112, 150);
    colors[ImGuiCol_FrameBg] = ImColor(247, 202, 208); 
    colors[ImGuiCol_TitleBg] = ImColor(249, 190, 199); 
    colors[ImGuiCol_TitleBgActive] = ImColor(255, 153, 172);
    colors[ImGuiCol_TitleBgCollapsed] = ImColor(249, 190, 199);
    colors[ImGuiCol_PopupBg] = ImColor(250, 224, 228);
    colors[ImGuiCol_ScrollbarBg] = ImColor(247, 202, 208);
    colors[ImGuiCol_ScrollbarGrab] = ImColor(255, 153, 172);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(255, 133, 161);
    colors[ImGuiCol_Border] = ImColor(255, 71, 126); 
    colors[ImGuiCol_Button] = ImColor(251, 177, 189); 
    colors[ImGuiCol_ButtonHovered] = ImColor(255, 133, 161);
    colors[ImGuiCol_ButtonActive] = ImColor(255, 112, 150); 
    colors[ImGuiCol_Text] = ImColor(0, 0, 0); 
    colors[ImGuiCol_Header] = ImColor(250, 224, 228);
    colors[ImGuiCol_HeaderActive] = ImColor(255, 153, 172); 
    colors[ImGuiCol_HeaderHovered] = ImColor(255, 133, 161); 
}

MyWindow::~MyWindow() {
    // Liberar los recursos de los objetos creados
    for (GameObject* obj : gameObjects) {
        delete obj; // Asegúrate de liberar la memoria
    }
    gameObjects.clear(); // Limpiar la lista


    SDL_GL_DeleteContext(_ctx);
    SDL_DestroyWindow(static_cast<SDL_Window*>(_window));
    ImGui_ImplSDL2_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
}

void MyWindow::selectObject(GameObject* obj) {
    if (selectedObject != obj) {
        selectedObject = obj;
    }
    else {
        selectedObject = nullptr; 
    }
}

void MyWindow::swapBuffers() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    static bool showAboutWindow = false; 
    static bool showGithubWindow = false;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Menu")) {
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

        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Sphere")) {
                GameObject::createPrimitive("Sphere", gameObjects);
            }
            if (ImGui::MenuItem("Cube")) {
                GameObject::createPrimitive("Cube", gameObjects);
            }
            if (ImGui::MenuItem("Cylinder")) {
                GameObject::createPrimitive("Cylinder", gameObjects);
            }
            if (ImGui::MenuItem("Cone")) {
                GameObject::createPrimitive("Cone", gameObjects);
            }
            if (ImGui::MenuItem("Torus")) {
                GameObject::createPrimitive("Torus", gameObjects);
            }
            if (ImGui::MenuItem("Plane")) {
                GameObject::createPrimitive("Plane", gameObjects);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();

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

    // Ventana anclada a la izquierda (Scene Objects)
    ImGui::SetNextWindowPos(ImVec2(0, 20));
    ImGui::SetNextWindowSizeConstraints(ImVec2(200, 100), ImVec2(400, _height - 100));
    ImGui::Begin("Scene Objects", nullptr, ImGuiWindowFlags_NoMove);

    if (!gameObjects.empty()) {
        for (GameObject* obj : gameObjects) {
            if (ImGui::Selectable(obj->getName().c_str(), selectedObject == obj)) {
                selectObject(obj);
            }
        }
    }
    else {
        ImGui::Text("No objects in the scene.");
    }
    ImGui::End();

    // Ventana anclada a la derecha (Inspector)
    ImGui::SetNextWindowPos(ImVec2(_width - 210, 20));
    ImGui::SetNextWindowSize(ImVec2(210, _height - 100)); // Tamaño fijo al inicio
    ImGui::SetNextWindowSizeConstraints(ImVec2(200, _height - 100), ImVec2(_width - 20, _height - 100)); // Tamaños mínimo y máximo
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoMove); // Ventana anclada a la derecha

    if (selectedObject) {
        ImGui::Text("Transform");

        glm::vec3 position = selectedObject->getPosition();
        glm::vec3 rotation = selectedObject->getRotation();
        glm::vec3 scale = selectedObject->getScale();

        if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f)) {
            selectedObject->setPosition(position); 
        }
        if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.1f)) {
            selectedObject->setRotation(rotation);
        }
        if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.1f, 0.1f, 10.0f)) {
            selectedObject->setScale(scale); 
        }
    }
    else {
        ImGui::Text("No GameObject Selected");
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(static_cast<SDL_Window*>(_window));
}