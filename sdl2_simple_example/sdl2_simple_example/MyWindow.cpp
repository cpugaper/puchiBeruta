#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include "MyWindow.h"
#include "Variables.h"
#include "Importer.h"
#include "Renderer.h"
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

static bool showAboutWindow = false;
static bool showGithubWindow = false;
static bool showConfig = false;

MyWindow::MyWindow(const std::string& title, int w, int h) : _width(w), _height(h), selectedObject(nullptr) {

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

MyWindow::~MyWindow() {
    // Liberar los recursos de los objetos creados
    for (GameObject* obj : gameObjects) {
        delete obj; // Aseg�rate de liberar la memoria
    }
    gameObjects.clear(); // Limpiar la lista


    SDL_GL_DeleteContext(_ctx);
    SDL_DestroyWindow(static_cast<SDL_Window*>(_window));
    ImGui_ImplSDL2_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
}

void MyWindow::configMyWindow()
{
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
    createHierarchyWindow();
    createInspectorWindow();
    createProjectWindow();
    createSceneWindow();

    ImGui::End();
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
            //ImGui::SetNextWindowPos(ImVec2(0, 20));
            ImGui::Begin("Config", nullptr);
            if (ImGui::CollapsingHeader("FPS Graph")) {
                ImGui::Text("FPS: %d", _fps); // Mostrar FPS actual
                // Mostrar el gr�fico de FPS en la ventana
                ImGui::PlotLines("FPS", _fpsHistory.data(), _fpsHistory.size(), 0, nullptr, 0.0f, 100.0f, ImVec2(0, 80));

            }

            ImGui::Separator();

            if (ImGui::CollapsingHeader("Window Settings")) {
                ImGui::InputInt("Width", &variables->windowWidth);
                ImGui::InputInt("Height", &variables->windowHeight);
                if (ImGui::Checkbox("Fullscreen", &variables->fullscreen)) {
                    // Actualiza la configuraci�n de fullscreen (si se cambia)
                }
                if (ImGui::Checkbox("V-Sync", &variables->vsyncEnabled)) {
                    // Actualiza la configuraci�n de V-Sync
                }

                // Aplicar cambios
                if (ImGui::Button("Apply Changes")) {
                    // Aqu� puedes aplicar los cambios de configuraci�n, como actualizar la ventana o el contexto OpenGL
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
            // Informaci�n de la memoria (ejemplo, en Windows)
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

                // Informaci�n de SDL
                SDL_version sdl_version;
                SDL_GetVersion(&sdl_version);
                ImGui::Text("SDL Version: %d.%d.%d", sdl_version.major, sdl_version.minor, sdl_version.patch);
                ImGui::Separator();
                // Informaci�n de OpenGL
                const char* glVersion = (const char*)glGetString(GL_VERSION);
                const char* glRenderer = (const char*)glGetString(GL_RENDERER);
                const char* glVendor = (const char*)glGetString(GL_VENDOR);
                ImGui::Text("OpenGL Version: %s", glVersion);
                ImGui::Text("OpenGL Renderer: %s", glRenderer);
                ImGui::Text("OpenGL Vendor: %s", glVendor);
                ImGui::Separator();

                //Informaci�n de DevIL (si est� integrado)
                std::string devilVersion = std::to_string(IL_VERSION).c_str();
                //const char* devilVersion = (const char*)ilGetString(IL_VERSION);
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

}

void MyWindow::createHierarchyWindow()
{
    ImGui::Begin("Hierarchy", nullptr);
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
}

void MyWindow::createInspectorWindow()
{
    ImGui::Begin("Inspector", nullptr);
    if (selectedObject) {
        // Transform information
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

        // Mesh information
        MeshData* meshData = selectedObject->getMeshData();
        if (meshData) {
            ImGui::Separator();
            ImGui::Text("Mesh Information");

            ImGui::Text("Vertices: %d", meshData->vertices.size() / 3);
            ImGui::Text("Indices: %d", meshData->indices.size() / 3);

            // Visualize normals
            bool showNormals = false;
            if (ImGui::CollapsingHeader("Show Normals")) {
                showNormals = true;

                if (meshData->normals.size() > 0) {
                    ImGui::Text("Normals by Triangle:");
                    for (size_t i = 0; i < meshData->normals.size(); i += 3) {
                        glm::vec3 normal = glm::vec3(meshData->normals[i], meshData->normals[i + 1], meshData->normals[i + 2]);
                        ImGui::Text("Normal %d: %.3f, %.3f, %.3f", i / 3, normal.x, normal.y, normal.z);
                    }
                }

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
    ImGui::Separator();
    ImGui::TextWrapped("Object Texture: %s", variables->textureFilePath.c_str());
    if (ImGui::Button("Checked Texture")) {
        GLuint newTextureID = importer.loadTexture(variables->checkerTexture);
        variables->window->selectedObject->textureID = newTextureID;
    }
    ImGui::End();
}

void MyWindow::createProjectWindow()
{
    ImGui::Begin("Project", nullptr);
    ImGui::Text("Assets list goes here");
    ImGui::End();
}

void MyWindow::createSceneWindow()
{
    int sceneWidth = 1280;
    int sceneHeight = 960;

    createFrameBuffer(sceneWidth, sceneHeight);

    ImGui::Begin("Scene", nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render(variables->window->gameObjects);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    ImGui::Image((void*)(intptr_t)textureColorbuffer, ImVec2(sceneWidth, sceneHeight));

    ImGui::End();
}

void MyWindow::swapBuffers() {
    // Calculate FPS
    _currentTime = SDL_GetTicks();
    _frameCount++;

    if (_currentTime - _lastTime >= 1000) {
        _fps = _frameCount;
        _frameCount = 0;
        _lastTime = _currentTime;

        // Almacenar FPS en la historia para el gr�fico
        if (_fpsHistory.size() >= 100) { // Limitar a 100 FPS en la historia
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