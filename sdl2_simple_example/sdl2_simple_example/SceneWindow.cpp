#include "SceneWindow.h"
#include "Renderer.h"
#include "Variables.h"
#include "imgui.h"

#include <GL/glew.h>
#include "Ray.h"
#include <iostream>
#include "Camera.h"
#include "ConsoleWindow.h"
#include "MyWindow.h"
#include "SimulationManager.h"

// Extern variables used in the main code
extern Renderer renderer;
extern Camera camera;

void SceneWindow::render() {
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    bool active = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    isActive = active;
    if (active) {
        console.addLog("SceneWindow active");
    }

    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize(); 

    ImVec2 controlPanelSize = ImVec2(windowSize.x, 40); 
    ImVec2 controlPanelPos = ImVec2(windowPos.x, windowPos.y + 20); 

    ImGui::SetNextWindowPos(controlPanelPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(controlPanelSize, ImGuiCond_Always);

    ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

    float windowWidth = ImGui::GetWindowWidth();
    float buttonWidth = 50.0f;
    float totalButtonWidth = 3 * buttonWidth + 2 * ImGui::GetStyle().ItemSpacing.x;
    float spacing = (windowWidth - totalButtonWidth) / 2.0f;

    ImGui::Dummy(ImVec2(spacing, 0));
    ImGui::SameLine();

    if (ImGui::Button("Start", ImVec2(buttonWidth, 0))) {
        SimulationManager::simulationManager.startSimulation(variables->window->gameObjects);
    }

    ImGui::SameLine();

    if (ImGui::Button("Pause", ImVec2(buttonWidth, 0))) {
        SimulationManager::simulationManager.pauseSimulation(variables->window->gameObjects);
    }

    ImGui::SameLine();

    if (ImGui::Button("Stop", ImVec2(buttonWidth, 0))) {
        SimulationManager::simulationManager.stopSimulation(variables->window->gameObjects);
    }

    ImGui::End();

    updateSceneSize();
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    if (SDL_MOUSEBUTTONDOWN == SDL_BUTTON_LEFT) {
        checkRaycast(mouseX, mouseY, framebufferWidth, framebufferHeight);
    }

    // Show texture on screen
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    ImGui::Image((void*)(intptr_t)textureColorbuffer, ImVec2(framebufferWidth, framebufferHeight));

    // Drag & Drop Management
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LibraryFile")) {
            const char* droppedFilePath = (const char*)payload->Data;
            if (droppedFilePath) {
                renderer.HandleDroppedFile(droppedFilePath);
            }
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::End();
}

void SceneWindow::updateSceneSize() {
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    int newWidth = static_cast<int>(availableSize.x);
    int newHeight = static_cast<int>(availableSize.y);

    if (newWidth != framebufferWidth || newHeight != framebufferHeight) {
        framebufferWidth = newWidth;
        framebufferHeight = newHeight;
        renderer.createFrameBuffer(framebufferWidth, framebufferHeight);
    }
}

Ray SceneWindow::getRayFromMouse(int mouseX, int mouseY, int screenWidth, int screenHeight) {
    console.addLog("Entra funcion getrayfrommouse");
    console.addLog("...");

    // Convertir las coordenadas del mouse a coordenadas normalizadas (NDC)
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;  // Invertimos el eje Y

    // Crear la matriz de proyecci�n y vista
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

    // La c�mara debe mirar en la direcci�n de su "frente" (puedes ajustar esto)
    glm::vec3 cameraFront = camera.position + glm::vec3(0.0f, 0.0f, 1.0f);  // Direcci�n hacia donde mira la c�mara
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);  // Vector "arriba" de la c�mara

    // Generamos la matriz de vista usando lookAt
    glm::mat4 view = glm::lookAt(camera.position, cameraFront, up);

    // Multiplicamos la matriz de proyecci�n por la de vista, y luego la invertimos
    glm::mat4 inverseProjectionView = glm::inverse(projection * view);

    // Proyectar el punto del rat�n hacia un rayo en el espacio 3D
    glm::vec4 clipSpacePos(x, y, -1.0f, 1.0f);
    glm::vec4 worldPos = inverseProjectionView * clipSpacePos;

    // Calcular la direcci�n del rayo
    glm::vec3 rayDirection = glm::normalize(glm::vec3(worldPos) - camera.position);

    // Devolver el rayo calculado
    return Ray(camera.position, rayDirection);
}

// M�todo que verifica si el rayo interseca con alg�n objeto en la escena
void SceneWindow::checkRaycast(int mouseX, int mouseY, int screenWidth, int screenHeight) {
    Ray ray = getRayFromMouse(mouseX, mouseY, screenWidth, screenHeight);

    for (auto& obj : variables->window->gameObjects) {
        console.addLog("HAY ESTOS OBJETOS EN LA ESCENA: " + obj->getName()); 
        MeshData* meshData = obj->getMeshData();
        if (meshData) {
            for (size_t i = 0; i < meshData->indices.size(); i += 3) {
                glm::vec3 vertex1 = glm::vec3(meshData->vertices[meshData->indices[i] * 3], meshData->vertices[meshData->indices[i] * 3 + 1], meshData->vertices[meshData->indices[i] * 3 + 2]);
                glm::vec3 vertex2 = glm::vec3(meshData->vertices[meshData->indices[i + 1] * 3], meshData->vertices[meshData->indices[i + 1] * 3 + 1], meshData->vertices[meshData->indices[i + 1] * 3 + 2]);
                glm::vec3 vertex3 = glm::vec3(meshData->vertices[meshData->indices[i + 2] * 3], meshData->vertices[meshData->indices[i + 2] * 3 + 1], meshData->vertices[meshData->indices[i + 2] * 3 + 2]);

                glm::vec3 edge1 = vertex2 - vertex1;
                glm::vec3 edge2 = vertex3 - vertex1;
                glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

                /* ImGui::Text("Triangle %d Normal: %.3f, %.3f, %.3f", i / 3, faceNormal.x, faceNormal.y, faceNormal.z);

                 std::string normalKey = std::to_string(faceNormal.x) + "," + std::to_string(faceNormal.y) + "," + std::to_string(faceNormal.z);
 */

                float t = 0.0f;
                if (ray.intersectsTriangle(vertex1, vertex2, vertex3, t)) {
                    variables->window->selectObject(obj);
                    console.addLog("Objeto seleccionado: " + variables->window->selectedObject->name);
                    /* char nameBuffer[256];
                     strncpy_s(nameBuffer, variables->window->selectedObject->name.c_str(), sizeof(nameBuffer) - 1);
                     if (ImGui::InputText("Object Name", nameBuffer, sizeof(nameBuffer))) {
                         variables->window->selectedObject->name = std::string(nameBuffer);
                     }*/
                    console.addLog("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA ");
                    variables->window->selectedObjects.push_back(obj);
                    break;
                }
            }
        }
    }
}