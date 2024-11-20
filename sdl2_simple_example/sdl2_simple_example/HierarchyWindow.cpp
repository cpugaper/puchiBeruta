#include "HierarchyWindow.h"

void HierarchyWindow::render(std::vector<GameObject*>& gameObjects, GameObject*& selectedObject) {
    ImGui::Begin("Hierarchy", nullptr);
    if (!gameObjects.empty()) {
        for (GameObject* obj : gameObjects) {
            if (ImGui::Selectable(obj->getName().c_str(), selectedObject == obj)) {
                selectedObject = obj;
            }
        }
    }
    else {
        ImGui::Text("No objects in the scene.");
    }
    ImGui::End();
}
