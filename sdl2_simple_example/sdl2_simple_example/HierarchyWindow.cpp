#include "HierarchyWindow.h"
#include "SimulationManager.h"
#include <algorithm>
#include <functional>

namespace {
    const ImVec4 SELECTED_PRIMARY_COLOR(0.5f, 0.0f, 0.5f, 1.0f);
    const ImVec4 SELECTED_SECONDARY_COLOR(0.7f, 0.3f, 0.7f, 1.0f);
    const ImVec4 DEFAULT_COLOR(0.3f, 0.3f, 0.3f, 1.0f);
}

void HierarchyWindow::deleteSelectedObjects(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject) {
    if (SimulationManager::simulationManager.getState() == SimulationManager::SimulationState::Running) {
        ImGui::OpenPopup("Error");
        return;
    }

    std::vector<GameObject*> objectsToDelete;
    for (auto* obj : selectedObjects) {
        if (!obj->parent || std::find(selectedObjects.begin(), selectedObjects.end(), obj->parent) == selectedObjects.end()) {
            objectsToDelete.push_back(obj);
        }
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
    ImGui::Begin("Hierarchy", nullptr);

    if (SimulationManager::simulationManager.getState() == SimulationManager::SimulationState::Running) {
        if (deleteKeyPressed) {
            ImGui::OpenPopup("Error");
        }
    }

    const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
    handleKeyboardInput(keyboardState, gameObjects, selectedObjects, selectedObject);

    if (!gameObjects.empty()) {
        renderHierarchyTree(gameObjects, selectedObjects, selectedObject, keyboardState);
    }
    else {
        ImGui::Text("No objects in the scene.");
    }

    ImGui::End();

    processNewObjects(gameObjects);
    handleParenting(selectedObjects);
    applyTransforms(gameObjects, selectedObjects);
    renderErrorPopup();
}

void HierarchyWindow::handleKeyboardInput(const Uint8* keyboardState, std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject) {
    if (keyboardState[SDL_SCANCODE_DELETE]) {
        deleteKeyPressed = true;
        deleteSelectedObjects(gameObjects, selectedObjects, selectedObject);
    }
    else {
        deleteKeyPressed = false;
    }
}

void HierarchyWindow::renderHierarchyTree(const std::vector<GameObject*>& gameObjects,
    std::vector<GameObject*>& selectedObjects,
    GameObject*& selectedObject,
    const Uint8* keyboardState) {
    std::function<void(GameObject*)> renderGameObject = [&](GameObject* obj) {
        // Usar el UUID como identificador único
        std::string uniqueLabel = obj->getName() + "##" + obj->getUUID();

        bool isSelected = std::find(selectedObjects.begin(), selectedObjects.end(), obj) != selectedObjects.end();

        ImGui::PushStyleColor(ImGuiCol_Header,
            isSelected && selectedObject == obj ? SELECTED_PRIMARY_COLOR :
            isSelected ? SELECTED_SECONDARY_COLOR : DEFAULT_COLOR);

        if (ImGui::Selectable(uniqueLabel.c_str(), isSelected)) {
            handleObjectSelection(obj, gameObjects, selectedObjects, selectedObject, keyboardState);
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

    // Renderizar solo objetos raíz (sin padre)
    for (GameObject* obj : gameObjects) {
        if (!obj->parent) {
            renderGameObject(obj);
        }
    }
}

void HierarchyWindow::handleObjectSelection(GameObject* obj,
    const std::vector<GameObject*>& gameObjects,
    std::vector<GameObject*>& selectedObjects,
    GameObject*& selectedObject,
    const Uint8* keyboardState) {
    bool ctrlPressed = keyboardState[SDL_SCANCODE_LCTRL] || keyboardState[SDL_SCANCODE_RCTRL];
    bool shiftPressed = keyboardState[SDL_SCANCODE_LSHIFT] || keyboardState[SDL_SCANCODE_RSHIFT];

    if (ctrlPressed) {
        // Multi-selección con Ctrl
        auto it = std::find(selectedObjects.begin(), selectedObjects.end(), obj);
        if (it != selectedObjects.end()) {
            selectedObjects.erase(it);
            if (selectedObject == obj) {
                selectedObject = selectedObjects.empty() ? nullptr : selectedObjects.back();
            }
        }
        else {
            selectedObjects.push_back(obj);
            selectedObject = obj;
        }
    }
    else if (shiftPressed && !selectedObjects.empty()) {
        // Encontrar todos los objetos del mismo nivel jerárquico
        std::vector<GameObject*> siblingObjects;
        GameObject* parent = obj->parent;

        if (parent) {
            siblingObjects = parent->children;
        }
        else {
            // Si no tiene padre, usar objetos raíz
            std::copy_if(gameObjects.begin(), gameObjects.end(), std::back_inserter(siblingObjects),
                [](GameObject* go) { return go->parent == nullptr; });
        }

        // Encontrar índices del último objeto seleccionado y el objeto actual
        auto getIndex = [&siblingObjects](GameObject* target) {
            return std::distance(siblingObjects.begin(),
                std::find(siblingObjects.begin(), siblingObjects.end(), target));
            };

        auto lastSelected = selectedObjects.back();
        size_t startIdx = getIndex(lastSelected);
        size_t endIdx = getIndex(obj);

        if (startIdx != siblingObjects.size() && endIdx != siblingObjects.size()) {
            if (startIdx > endIdx) std::swap(startIdx, endIdx);

            selectedObjects.clear();
            for (size_t i = startIdx; i <= endIdx; ++i) {
                selectedObjects.push_back(siblingObjects[i]);
            }
            selectedObject = obj;
        }
    }
    else {
        // Selección simple
        selectedObjects.clear();
        selectedObjects.push_back(obj);
        selectedObject = obj;
    }
}

void HierarchyWindow::handleParenting(std::vector<GameObject*>& selectedObjects) {
    const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);

    if (keyboardState[SDL_SCANCODE_P]) {
        if (!pKeyPressed) {
            pKeyPressed = true;
            processParenting(selectedObjects);
        }
    }
    else {
        pKeyPressed = false;
    }
}

void HierarchyWindow::processParenting(std::vector<GameObject*>& selectedObjects) {
    if (selectedObjects.size() == 1) {
        GameObject* parent = selectedObjects[0];
        std::vector<GameObject*> childrenCopy = parent->children;
        for (GameObject* child : childrenCopy) {
            parent->removeChild(child);
        }
    }
    else if (selectedObjects.size() > 1) {
        GameObject* parent = selectedObjects.back();
        selectedObjects.pop_back();

        for (GameObject* child : selectedObjects) {
            parent->addChild(child);
        }

        parent->updateChildTransforms();
        selectedObjects = { parent };
    }
}

void HierarchyWindow::applyTransforms(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects) {
    if (selectedObjects.empty()) return;

    GameObject* lastSelected = selectedObjects.back();
    if (!lastSelected->parent) {
        for (GameObject* obj : selectedObjects) {
            obj->updateChildTransforms();
        }
    }
    else {
        lastSelected->updateChildTransforms();
    }
}

void HierarchyWindow::processNewObjects(std::vector<GameObject*>& gameObjects) {
    std::vector<GameObject*> newObjects = getNewObjects(gameObjects);
    if (!newObjects.empty()) {
        setupInitialHierarchy(newObjects);
    }
    lastKnownObjects = gameObjects;
}

std::vector<GameObject*> HierarchyWindow::getNewObjects(const std::vector<GameObject*>& currentObjects) {
    std::vector<GameObject*> newObjects;

    for (GameObject* obj : currentObjects) {
        if (!obj->parent && std::find(lastKnownObjects.begin(), lastKnownObjects.end(), obj) == lastKnownObjects.end()) {
            std::vector<GameObject*> modelObjects = { obj };

            for (GameObject* potentialChild : currentObjects) {
                if (potentialChild != obj &&
                    !potentialChild->parent &&
                    std::find(lastKnownObjects.begin(), lastKnownObjects.end(), potentialChild) == lastKnownObjects.end()) {
                    modelObjects.push_back(potentialChild);
                }
            }

            newObjects.insert(newObjects.end(), modelObjects.begin(), modelObjects.end());
        }
    }

    return newObjects;
}

void HierarchyWindow::setupInitialHierarchy(std::vector<GameObject*>& gameObjects) {
    if (gameObjects.empty() || gameObjects[0]->fromScene) return;

    GameObject* parent = gameObjects[0];
    if (parent->parent) return;

    for (size_t i = 1; i < gameObjects.size(); ++i) {
        GameObject* child = gameObjects[i];
        if (!child->parent && child != parent) {
            parent->addChild(child);
        }
    }
    parent->updateChildTransforms();
}

void HierarchyWindow::renderErrorPopup() {
    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Object can't be deleted in running simulation. Stop the simulation to do so.");
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}