#include "HierarchyWindow.h"
#include "SimulationManager.h"
#include <SDL2/SDL_events.h>

void HierarchyWindow::deleteSelectedObjects(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject) {
    if (SimulationManager::simulationManager.getState() == SimulationManager::SimulationState::Running) {
        ImGui::OpenPopup("Error");
        return;
    }
    
    std::vector<GameObject*> objectsToDelete = selectedObjects;

    for (auto it = objectsToDelete.begin(); it != objectsToDelete.end();) {
        GameObject* obj = *it;
        if (obj->parent && std::find(objectsToDelete.begin(), objectsToDelete.end(), obj->parent) != objectsToDelete.end()) {
            it = objectsToDelete.erase(it);
            continue;
        }
        ++it;
    }

    for (GameObject* obj : objectsToDelete) {
        if (obj == selectedObject) {
            selectedObject = nullptr; 
        }

        if (obj->parent) {
            obj->parent->removeChild(obj);
        }
        deleteObjectAndChildren(obj, gameObjects);
    }
    selectedObjects.clear();
}

void HierarchyWindow::deleteObjectAndChildren(GameObject* obj, std::vector<GameObject*>& gameObjects) {
    std::vector<GameObject*> childrenCopy = obj->children;
    for (GameObject* child : childrenCopy) {
        deleteObjectAndChildren(child, gameObjects);
    }
    obj->children.clear();
    gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), obj), gameObjects.end());
    delete obj;
}

void HierarchyWindow::render(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject) {
    if (SimulationManager::simulationManager.getState() == SimulationManager::SimulationState::Running) {
        if (deleteKeyPressed) {
            ImGui::OpenPopup("Error");
        }
    }
    
    ImGui::Begin("Hierarchy", nullptr);

    if (!gameObjects.empty()) {

        const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);

        if (keyboardState[SDL_SCANCODE_DELETE]) {
            deleteKeyPressed = true;
            deleteSelectedObjects(gameObjects, selectedObjects, selectedObject);
        }
        else {
            deleteKeyPressed = false; 
        }

        std::function<void(GameObject*)> renderGameObject = [&](GameObject* obj) {

            bool isSelected = (std::find(selectedObjects.begin(), selectedObjects.end(), obj) != selectedObjects.end());
            bool isParent = !obj->children.empty();

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

            bool isItemClicked = ImGui::Selectable(obj->getName().c_str(), isSelected);

            if (isItemClicked) {
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

            ImGui::PopStyleColor();

            if (!obj->children.empty()) {
                ImGui::Indent();
                for (GameObject* child : obj->children) {
                    renderGameObject(child);
                }
                ImGui::Unindent();
            }
        };

        for (GameObject* obj : gameObjects) {
            if (obj->parent == nullptr) {
                renderGameObject(obj);
            }
        }
    }
    else {
        ImGui::Text("No objects in the scene.");
    }
    ImGui::End();

    std::vector<GameObject*> newObjects = getNewObjects(gameObjects);
    if (!newObjects.empty()) {
        setupInitialHierarchy(newObjects);
    }
    lastKnownObjects = gameObjects;

    handleParenting(selectedObjects);
    applyTransforms(gameObjects, selectedObjects);

    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Object can't be deleted in running simulation. Stop the simulation to do so.");
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup(); 
        }
        ImGui::EndPopup();
    }
}

void HierarchyWindow::handleParenting(std::vector<GameObject*>& selectedObjects) {
    const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);

    if (keyboardState[SDL_SCANCODE_P]) {

        if (!pKeyPressed) {
            pKeyPressed = true;

            // Case 1: Undo parent-child relation 
            if (selectedObjects.size() == 1) {
                GameObject* parent = selectedObjects[0];
                if (!parent->children.empty()) {
                    std::vector<GameObject*> childrenCopy = parent->children;
                    for (GameObject* child : childrenCopy) {
                        parent->removeChild(child);
                    }
                }
            }
            // Case 2: Create parent-child relation
            else if (selectedObjects.size() > 1) {
                GameObject* potentialParent = selectedObjects.back();
                selectedObjects.pop_back();

                for (GameObject* child : selectedObjects) {
                    potentialParent->addChild(child);
                }

                potentialParent->updateChildTransforms();
                selectedObjects.clear();
                selectedObjects.push_back(potentialParent);
            }
        }
    }
    else {
        pKeyPressed = false; 
    }
}

void HierarchyWindow::applyTransforms(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects) {
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

std::vector<GameObject*> HierarchyWindow::getNewObjects(const std::vector<GameObject*>& currentObjects) {
    std::vector<GameObject*> newObjects;

    for (GameObject* obj : currentObjects) {
        bool isNew = std::find(lastKnownObjects.begin(), lastKnownObjects.end(), obj) == lastKnownObjects.end();
        if (isNew && obj->parent == nullptr) {
            std::vector<GameObject*> modelObjects;
            modelObjects.push_back(obj);

            for (GameObject* potentialChild : currentObjects) {
                if (potentialChild != obj &&
                    std::find(lastKnownObjects.begin(), lastKnownObjects.end(), potentialChild) == lastKnownObjects.end() &&
                    potentialChild->parent == nullptr) {
                    modelObjects.push_back(potentialChild);
                }
            }

            if (!modelObjects.empty()) {
                newObjects.insert(newObjects.end(), modelObjects.begin(), modelObjects.end());
            }
        }
    }

    return newObjects;
}

void HierarchyWindow::setupInitialHierarchy(std::vector<GameObject*>& gameObjects) {
    if (gameObjects.empty()) return;

    GameObject* parent = gameObjects[0];

    if (parent->parent != nullptr) return;

    for (size_t i = 1; i < gameObjects.size(); ++i) {
        GameObject* child = gameObjects[i];

        if (child->parent == nullptr && child != parent) {
            parent->addChild(child);
        }
    }
    parent->updateChildTransforms();
}