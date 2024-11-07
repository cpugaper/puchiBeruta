#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include "MyWindow.h"
#include "Variables.h"
#include "Importer.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "GameObject.h"
#include <SDL2/SDL_opengl.h>
#include <vector>
#include <iostream>
#include <fstream>

#include <IL/il.h>
#include <IL/ilu.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/sysctl.h>  // To obtain memory in macOS
#include <unistd.h>      // To obtain memory in Linux
#endif

extern Importer importer;
extern std::vector<GameObject*> gameObjects;
extern MyWindow* window;

ImGuiIO* g_io = nullptr;

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

    g_io = &ImGui::GetIO();
    g_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard controls
    g_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking
    g_io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable multiple viewports

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
        objectSelected = true;
    }
    else {
        selectedObject = nullptr;
        objectSelected = false;
    }
}

void MyWindow::deleteSelectedObject() {
    if (selectedObject) {
        auto it = std::find(gameObjects.begin(), gameObjects.end(), selectedObject);
        if (it != gameObjects.end()) {
            delete* it;
            gameObjects.erase(it);
        }
        selectedObject = nullptr;
        objectSelected = false;
    }
}

void MyWindow::swapBuffers() {
    // Calculate FPS
    _currentTime = SDL_GetTicks(); 
    _frameCount++; 

    if (_currentTime - _lastTime >= 1000) {
        _fps = _frameCount;
        _frameCount = 0;
        _lastTime = _currentTime;

        // Almacenar FPS en la historia para el gráfico
        if (_fpsHistory.size() >= 100) { // Limitar a 100 FPS en la historia
            _fpsHistory.erase(_fpsHistory.begin());
        }
        _fpsHistory.push_back((float)_fps);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    static bool showAboutWindow = false;
    static bool showGithubWindow = false;
    static bool showConfig = false;

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("Dockspace", NULL, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar);

    ImGuiID dockspaceId = ImGui::GetID("Dockspace");
    ImGui::DockSpace(ImGui::GetMainViewport()->ID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

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

        if (ImGui::BeginMenu("Project")) {
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
        if (ImGui::BeginMenu("Configuration")) {
            showConfig = !showConfig;

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();

        if (showConfig) {
            //ImGui::SetNextWindowPos(ImVec2(0, 20));
            ImGui::Begin("Config", nullptr);
            if (ImGui::CollapsingHeader("FPS Graph")) {
                ImGui::Text("FPS: %d", _fps); // Mostrar FPS actual
                // Mostrar el gráfico de FPS en la ventana
                ImGui::PlotLines("FPS", _fpsHistory.data(), _fpsHistory.size(), 0, nullptr, 0.0f, 100.0f, ImVec2(0, 80));

            }

            ImGui::Separator();

            if (ImGui::CollapsingHeader("Window Settings")) {
                ImGui::InputInt("Width", &variables->windowWidth);
                ImGui::InputInt("Height", &variables->windowHeight);
                if (ImGui::Checkbox("Fullscreen", &variables->fullscreen)) {
                    // Actualiza la configuración de fullscreen (si se cambia)
                }
                if (ImGui::Checkbox("V-Sync", &variables->vsyncEnabled)) {
                    // Actualiza la configuración de V-Sync
                }

                // Aplicar cambios
                if (ImGui::Button("Apply Changes")) {
                    // Aquí puedes aplicar los cambios de configuración, como actualizar la ventana o el contexto OpenGL
                    SDL_SetWindowSize(_window, variables->windowWidth, variables->windowHeight);
                    if (variables->fullscreen) {
                        SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    }
                    else {
                        SDL_SetWindowFullscreen(_window, 0); // Desactivar fullscreen
                    }
                }
            }

            ImGui::Separator();

            if (ImGui::CollapsingHeader("Texture Settings")) {
                ImGui::SliderFloat("Texture Filter Quality", &variables->textureFilterQuality, 0.0f, 2.0f);
                ImGui::SliderFloat("Anisotropic Filter", &variables->textureAnisotropicLevel, 1.0f, 16.0f);
            }

            ImGui::Separator();
            // Información de la memoria (ejemplo, en Windows)
            if (ImGui::CollapsingHeader("Info")) {
                // (esto depende del sistema operativo)
                MEMORYSTATUSEX statex;
                statex.dwLength = sizeof(statex);
                GlobalMemoryStatusEx(&statex);

                ImGui::Text("Memory Usage:");
                ImGui::Text("Total RAM: %.2f GB", statex.ullTotalPhys / (1024.0 * 1024.0 * 1024.0));
                ImGui::Text("Available RAM: %.2f GB", statex.ullAvailPhys / (1024.0 * 1024.0 * 1024.0));
                ImGui::Text("Used RAM: %.2f GB", (statex.ullTotalPhys - statex.ullAvailPhys) / (1024.0 * 1024.0 * 1024.0));

                ImGui::Separator();

                // Información de SDL
                SDL_version sdl_version;
                SDL_GetVersion(&sdl_version);
                ImGui::Text("SDL Version: %d.%d.%d", sdl_version.major, sdl_version.minor, sdl_version.patch);
                ImGui::Separator();
                // Información de OpenGL
                const char* glVersion = (const char*)glGetString(GL_VERSION);
                const char* glRenderer = (const char*)glGetString(GL_RENDERER);
                const char* glVendor = (const char*)glGetString(GL_VENDOR);
                ImGui::Text("OpenGL Version: %s", glVersion);
                ImGui::Text("OpenGL Renderer: %s", glRenderer);
                ImGui::Text("OpenGL Vendor: %s", glVendor);
                ImGui::Separator();

                //Información de DevIL (si está integrado)
                wchar_t* devilVersion = iluGetString(IL_VERSION);
                ImGui::Text("DevIL Version: %s", devilVersion);

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



        // Mostrar la información de la malla
        MeshData* meshData = selectedObject->getMeshData();
        if (meshData) {
            ImGui::Separator();
            ImGui::Text("Mesh Information");

            ImGui::Text("Vertices: %d", meshData->vertices.size() / 3);  // Asumimos que cada vértice tiene 3 componentes (x, y, z)
            ImGui::Text("Indices: %d", meshData->indices.size() / 3);   // Asumimos que cada cara tiene 3 índices (triángulos)

            // Opción para visualizar las normales
            bool showNormals = false;
            if (ImGui::CollapsingHeader("Show Normals")) {
                showNormals = true;

                // Mostrar normales por triángulo
                if (meshData->normals.size() > 0) {
                    ImGui::Text("Normals by Triangle:");
                    for (size_t i = 0; i < meshData->normals.size(); i += 3) {
                        glm::vec3 normal = glm::vec3(meshData->normals[i], meshData->normals[i + 1], meshData->normals[i + 2]);
                        ImGui::Text("Normal %d: %.3f, %.3f, %.3f", i / 3, normal.x, normal.y, normal.z);
                    }
                }

                // Opción para ver las normales por cara (media de las normales de triángulos)
                if (meshData->vertices.size() / 3 > 0) {
                    ImGui::Text("Normals by Face:");
                    for (size_t i = 0; i < meshData->indices.size(); i += 3) {
                        glm::vec3 vertex1 = glm::vec3(meshData->vertices[meshData->indices[i] * 3], meshData->vertices[meshData->indices[i] * 3 + 1], meshData->vertices[meshData->indices[i] * 3 + 2]);
                        glm::vec3 vertex2 = glm::vec3(meshData->vertices[meshData->indices[i + 1] * 3], meshData->vertices[meshData->indices[i + 1] * 3 + 1], meshData->vertices[meshData->indices[i + 1] * 3 + 2]);
                        glm::vec3 vertex3 = glm::vec3(meshData->vertices[meshData->indices[i + 2] * 3], meshData->vertices[meshData->indices[i + 2] * 3 + 1], meshData->vertices[meshData->indices[i + 2] * 3 + 2]);

                        // Calcular la normal de la cara usando el producto cruzado
                        glm::vec3 edge1 = vertex2 - vertex1;
                        glm::vec3 edge2 = vertex3 - vertex1;
                        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

                        ImGui::Text("Face %d Normal: %.3f, %.3f, %.3f", i / 3, faceNormal.x, faceNormal.y, faceNormal.z);
                    }
                }
            }
        }
    }
    else {
        ImGui::Text("No GameObject Selected");
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (g_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(_window, _ctx);
    }

    SDL_GL_SwapWindow(static_cast<SDL_Window*>(_window));
}