#include "SceneWindow.h"
#include "Renderer.h"
#include "Variables.h"
#include "imgui.h"

// Extern variables used in the main code
extern Renderer renderer;

void SceneWindow::render() {
    ImGui::Begin("Scene", nullptr);

    updateSceneSize();

    // Mostrar la textura de la escena
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    ImGui::Image((void*)(intptr_t)textureColorbuffer, ImVec2(framebufferWidth, framebufferHeight));

    // Drag & Drop Management
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetFile")) {
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
