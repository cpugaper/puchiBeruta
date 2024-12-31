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
        //console.addLog("SceneWindow active");
    }

    windowPos = ImGui::GetWindowPos();
    windowSize = ImGui::GetWindowSize(); 
    contentPos = ImGui::GetCursorScreenPos();
    contentRegionAvail = ImGui::GetContentRegionAvail();

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
   /* int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    if (SDL_MOUSEBUTTONDOWN == SDL_BUTTON_LEFT) {
        checkRaycast(mouseX, mouseY, framebufferWidth, framebufferHeight);
    }*/
    if (rayoexists) {
        DrawRay(*rayo, 1000);
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
    console.addLog("Entra funcion getRayFromMouse");

    // Ajustar las coordenadas del mouse considerando:
    // 1. La posición de la ventana
    // 2. El espacio del header de ImGui
    // 3. El panel de control superior
    float headerOffset = 25.0f;  // Height of the ImGui window header
    float controlPanelHeight = 0.0f;  // Height of your control panel

    mouseX -= static_cast<int>(windowPos.x);
    mouseY -= static_cast<int>(windowPos.y + headerOffset + controlPanelHeight);

    // Usar el tamaño del framebuffer para los cálculos
    float windowWidth = framebufferWidth;
    float windowHeight = framebufferHeight;

    // Verificar que el mouse está dentro de los límites
    if (mouseX < 0 || mouseX > windowWidth || mouseY < 0 || mouseY > windowHeight) {
        console.addLog("Mouse fuera de límites");
        return Ray(glm::vec3(0), glm::vec3(0));
    }

    // Convertir a NDC usando el tamaño del framebuffer
    float x = (2.0f * mouseX) / windowWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / windowHeight;

    // Crear la matriz de proyección usando el aspect ratio del framebuffer
    float aspectRatio = static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    // Calcular la dirección de la cámara
    /*glm::vec3 forward = glm::normalize(glm::vec3(
        -cos(glm::radians(camera.angleY)) * sin(glm::radians(camera.angleX)),
        -sin(glm::radians(camera.angleY)),
        -cos(glm::radians(camera.angleY)) * cos(glm::radians(camera.angleX))
    ));*/
    glm::vec3 forward = camera.getForwardVector(); 

    glm::mat4 view = glm::lookAt(
        -camera.position,
        -camera.position + forward,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 inverseProjectionView = glm::inverse(projection * view);

    glm::vec4 clipSpaceNear(x, y, -1.0f, 1.0f);
    glm::vec4 clipSpaceFar(x, y, 1.0f, 1.0f);

    glm::vec4 worldSpaceNear = inverseProjectionView * clipSpaceNear;
    glm::vec4 worldSpaceFar = inverseProjectionView * clipSpaceFar;

    worldSpaceNear /= worldSpaceNear.w;
    worldSpaceFar /= worldSpaceFar.w;

    glm::vec3 rayDirection = glm::normalize(
        glm::vec3(worldSpaceFar) - glm::vec3(worldSpaceNear)
    );

    console.addLog("Mouse pos (adjusted): " +
        std::to_string(mouseX) + ", " +
        std::to_string(mouseY));

    console.addLog("Ray origin: " +
        std::to_string(-camera.position.x) + ", " +
        std::to_string(-camera.position.y) + ", " +
        std::to_string(-camera.position.z));

    console.addLog("Ray direction: " +
        std::to_string(rayDirection.x) + ", " +
        std::to_string(rayDirection.y) + ", " +
        std::to_string(rayDirection.z));

    console.addLog("Camera rotation: " +
        std::to_string(camera.angleX) + ", " + 
        std::to_string(camera.angleY) + ", "); 

    console.addLog("...................");

    return Ray(-camera.position, rayDirection);
}
void SceneWindow::DrawRay(const Ray& ray, float length) {
    glm::vec3 endPoint = ray.origin + ray.direction * length;

    glPushMatrix();
    glColor3f(1.0f, 0.0f, 0.0f); // Rojo para el rayo

    // Dibuja el rayo
    glBegin(GL_LINES);
    glVertex3f(ray.origin.x, ray.origin.y, ray.origin.z);
    glVertex3f(endPoint.x, endPoint.y, endPoint.z);
    glEnd();

    // Dibuja el punto de origen
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glVertex3f(ray.origin.x, ray.origin.y, ray.origin.z);
    glVertex3f(endPoint.x, endPoint.y, endPoint.z);
    glEnd();

    glPopMatrix();
}
// M�todo que verifica si el rayo interseca con alg�n objeto en la escena
void SceneWindow::checkRaycast(int mouseX, int mouseY, int screenWidth, int screenHeight) {
    Ray ray = getRayFromMouse(mouseX, mouseY, screenWidth, screenHeight);
    rayo = new Ray(ray);
    rayo->origin = ray.origin;
    rayo->direction = ray.direction;

    rayoexists = true;
    for (auto& obj : variables->window->gameObjects) {
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
                    if (variables->window->selectedObject != nullptr) {
                        console.addLog("Objeto seleccionado: " + variables->window->selectedObject->name);
                    }
                    
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