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
            if (ImGui::MenuItem("Exit")) {
                SDL_Event quit_event;
                quit_event.type = SDL_QUIT;
                SDL_PushEvent(&quit_event);
            }
            if (ImGui::MenuItem("GitHub")) {
                showGithubWindow = true;
            }
            if (ImGui::MenuItem("About")) {
                showAboutWindow = true;
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
            ImGui::Text("Team: ");
            ImGui::Text("Maria Perarnau");
            ImGui::Text("Rebeca Fernández");
            ImGui::Text("Carla Puga");
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

    ImGui::Begin("Inspector");

    //GameObject* selectedObject = nullptr;

    if (selectedObject)
    {
        ImGui::Text("Transform");
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", selectedObject->getPosition().x, selectedObject->getPosition().y, selectedObject->getPosition().z);
        ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", selectedObject->getRotation().x, selectedObject->getRotation().y, selectedObject->getRotation().z);
        ImGui::Text("Scale: (%.2f, %.2f, %.2f)", selectedObject->getScale().x, selectedObject->getScale().y, selectedObject->getScale().z);
    }
    else {
        ImGui::Text("No GameObject Selected");
    }

    ImGui::End();

    //Ventana nueva para mostrar todos los objetos de la escena
    if (ImGui::Begin("Scene Objects")) {
        if (!gameObjects.empty()) {
            for (GameObject* obj : gameObjects) { // Usamos una referencia para evitar copias innecesarias 
                if (ImGui::Selectable(obj->getName().c_str(), selectedObject == obj)) { // Usa selectable para permitir la selección 
                    selectObject(obj); // Selecciona el objeto 
                }
            }
        }
        else {
            ImGui::Text("No objects in the scene.");
        }  
    }
    //ImGui::Begin("Scene Objects");
    //for (GameObject* obj : gameObjects) { // Iterar sobre los punteros a GameObject
    //    if (ImGui::Selectable(obj->getName().c_str(), selectedObject == obj)) {
    //        selectObject(obj);
    //    }
    //}
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(static_cast<SDL_Window*>(_window));
}