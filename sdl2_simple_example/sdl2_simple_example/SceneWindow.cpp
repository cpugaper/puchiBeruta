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

enum class ActiveButton { None, Start, Pause, Stop };
ActiveButton activeButton = ActiveButton::None;

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
    ImVec2 controlPanelPos = ImVec2(windowPos.x, windowPos.y + 30); 

    ImGui::SetNextWindowPos(controlPanelPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(controlPanelSize, ImGuiCond_Always);

    ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

    float buttonWidth = 50.0f;
    float buttonHeight = ImGui::GetFrameHeight();
    float spacingX = ImGui::GetStyle().ItemSpacing.x;
    float totalButtonWidth = 3 * buttonWidth + 2 * spacingX;

    float windowWidth = ImGui::GetWindowWidth();
    float panelSpacing = (windowWidth - totalButtonWidth) / 2.0f; 

    ImVec2 rectMin = ImVec2(windowPos.x + panelSpacing, controlPanelPos.y);
    ImVec2 rectMax = ImVec2(rectMin.x + totalButtonWidth + 15, controlPanelPos.y + buttonHeight);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(rectMin, rectMax, IM_COL32(50, 50, 50, 150));

    ImGui::Dummy(ImVec2(panelSpacing, 0));
    ImGui::SameLine();

    if (activeButton == ActiveButton::Start) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 1.0f, 1.0f)); 
    }
    
    if (activeButton == ActiveButton::Start) {
        ImGui::PopStyleColor();
    }
    if (ImGui::Button("Start", ImVec2(buttonWidth, 0)) && activeButton != ActiveButton::Start) { 
        activeButton = ActiveButton::Start;
        SimulationManager::simulationManager.startSimulation(variables->window->gameObjects);
        for (GameObject* obj : variables->window->gameObjects) {
            if (obj->isCamera) {
                camera.initPosition = camera.position;
                camera.initAngleX = camera.angleX;
                camera.initAngleY = camera.angleY;
                camera.position = -obj->getPosition();
                camera.angleX = -obj->getRotation().x;
                camera.angleY = -obj->getRotation().y;
                //camera.scale = obj->getScale(); 
            }
        }

    }
    ImGui::SameLine();

    if (activeButton == ActiveButton::Pause) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 1.0f, 1.0f)); 
    }
    if (ImGui::Button("Pause", ImVec2(buttonWidth, 0))) {
        activeButton = ActiveButton::Pause;
        SimulationManager::simulationManager.pauseSimulation(variables->window->gameObjects);
    }
    if (activeButton == ActiveButton::Pause) {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    if (activeButton == ActiveButton::Stop) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 1.0f, 1.0f)); 
    }
    
    if (activeButton == ActiveButton::Stop) {
        ImGui::PopStyleColor();
    }

    if (ImGui::Button("Stop", ImVec2(buttonWidth, 0))) {
        activeButton = ActiveButton::Stop;
        SimulationManager::simulationManager.stopSimulation(variables->window->gameObjects);
        for (GameObject* obj : variables->window->gameObjects) {
            if (obj->isCamera) {
                camera.position = camera.initPosition;
                camera.angleX = camera.initAngleX;
                camera.angleY = camera.initAngleY;

                /*camera.angleX = obj->getRotation();
                camera.scale = obj->getScale(); */
            }
        }
    }
    ImGui::End();

    updateSceneSize();
  
    if (rayoexists) {
        //DrawRay(*rayo, 1000);
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
glm::mat4 SceneWindow::ProjectionMatrix() { 
    float aspectRatio = static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.001f, 100.0f); 

    return projection;
}
glm::mat4 SceneWindow::ViewMatrix() {
    glm::vec3 forward = camera.getForwardVector();

    glm::mat4 view = glm::lookAt(
        -camera.position,
        -camera.position + forward,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    return view;
}
Ray SceneWindow::getRayFromMouse(int mouseX, int mouseY, int screenWidth, int screenHeight) {
    console.addLog("Entra funcion getRayFromMouse");

    // Ajustar las coordenadas del mouse considerando:
    // 1. La posición de la ventana
    // 2. El espacio del header de ImGui
    // 3. El panel de control superior
    float headerOffset = 0.0f;  // Height of the ImGui window header

    mouseX -= static_cast<int>(windowPos.x);
    mouseY -= static_cast<int>(windowPos.y + headerOffset);

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

    // Calcular la dirección de la cámara
    /*glm::vec3 forward = glm::normalize(glm::vec3(
        -cos(glm::radians(camera.angleY)) * sin(glm::radians(camera.angleX)),
        -sin(glm::radians(camera.angleY)),
        -cos(glm::radians(camera.angleY)) * cos(glm::radians(camera.angleX))
    ));*/

    glm::mat4 inverseProjectionView = glm::inverse(ProjectionMatrix() * ViewMatrix());

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

                    variables->window->selectedObjects.push_back(obj);
                    
                    break;
                }
            }
        }
    }
}