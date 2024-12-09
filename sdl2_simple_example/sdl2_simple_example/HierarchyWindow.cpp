#include "HierarchyWindow.h"
#include <SDL2/SDL_events.h>

std::vector<GameObject*> selectedObjects;

void HierarchyWindow::render(std::vector<GameObject*>& gameObjects, GameObject*& selectedObject) {
    ImGui::Begin("Hierarchy", nullptr);

    if (!gameObjects.empty()) {

        const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);

        for (GameObject* obj : gameObjects) {

            bool isSelected = (std::find(selectedObjects.begin(), selectedObjects.end(), obj) != selectedObjects.end());
            bool isParent = !obj->children.empty();
            bool isChild = obj->parent != nullptr;

            // Visual help 
            if (isSelected && selectedObject == obj) {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.5f, 0.0f, 0.5f, 1.0f)); // Last object selected 
            }
            else if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.7f, 0.3f, 0.7f, 1.0f)); // Selected objects
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.3f, 0.3f, 1.0f)); // Others
            }

            if (isChild) {
                ImGui::Text("  - %s", obj->getName().c_str()); 
            }
            else {
                if (ImGui::Selectable(obj->getName().c_str(), isSelected)) {
                    if (keyboardState[SDL_SCANCODE_LCTRL] || keyboardState[SDL_SCANCODE_RCTRL]) {
                        if (isSelected) {
                            selectedObjects.erase(std::remove(selectedObjects.begin(), selectedObjects.end(), obj), selectedObjects.end());
                        }
                        else {
                            selectedObjects.push_back(obj);
                        }
                    }
                    else {
                        selectedObjects.clear();
                        selectedObjects.push_back(obj);
                    }
                    selectedObject = obj; // Last selected object
                }
            }

            ImGui::PopStyleColor();  
        }
    }
    else {
        ImGui::Text("No objects in the scene.");
    }
    ImGui::End();

    handleParenting();
    applyTransforms(gameObjects);
}

void HierarchyWindow::handleParenting() {
    const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);

    if (keyboardState[SDL_SCANCODE_P]) {

        if (selectedObjects.size() > 1) {

            GameObject* parent = selectedObjects.back(); 
            selectedObjects.pop_back();

            for (GameObject* child : selectedObjects) {
                parent->addChild(child);
            }

            parent->updateChildTransforms();
            selectedObjects.clear();
            selectedObjects.push_back(parent); 
        }
    }
}

void HierarchyWindow::applyTransforms(std::vector<GameObject*>& gameObjects) {
    if (selectedObjects.empty()) return;

    GameObject* lastSelected = selectedObjects.back();

    if (lastSelected->parent == nullptr) {
        for (GameObject* obj : selectedObjects) {
            obj->updateChildTransforms();
        }
    }
    else {
        lastSelected->updateChildTransforms();
    }
}
