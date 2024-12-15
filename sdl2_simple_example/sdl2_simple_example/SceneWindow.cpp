#include "SceneWindow.h"
#include "Renderer.h"
#include "Variables.h"
#include "imgui.h"
#include "Camera.h"
#include "Ray.h"

// Extern variables used in the main code
extern Renderer renderer;
extern Camera camera;

void SceneWindow::render() {
    ImGui::Begin("Scene", nullptr);

    updateSceneSize();

    // Obtener la posición del ratón en la ventana
    ImVec2 mousePos = ImGui::GetMousePos();
    float width = framebufferWidth;
    float height = framebufferHeight;

    // Convertir la posición del ratón en un rayo en el espacio de la escena
    Ray ray = camera.getMouseRay(mousePos.x, mousePos.y, width, height);

    // Revisar si el rayo interseca alguno de los objetos cuando el ratón hace clic
    if (ImGui::IsMouseClicked(0)) { // 0 es el botón izquierdo del ratón
        for (GameObject* obj : variables->window->gameObjects) {
            float t = 0.0f;
            if (obj->intersectsRay(ray, t)) {
                // Si el rayo interseca el objeto, seleccionarlo
                variables->window->selectedObject = obj;
                console.addLog("Selected scene object");
                break; // Solo seleccionamos el primer objeto con el que interseca el rayo
            }
        }
    }

    // Mostrar la textura de la escena
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
