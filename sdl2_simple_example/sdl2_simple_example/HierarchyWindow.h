#ifndef HIERARCHY_WINDOW_H
#define HIERARCHY_WINDOW_H

#include "GameObject.h"
#include <vector>
#include <imgui.h>
#include <SDL2/SDL_events.h>

class HierarchyWindow {
public:
    void render(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject);

private:
    bool deleteKeyPressed = false;
    bool pKeyPressed = false;
    std::vector<GameObject*> lastKnownObjects;

    void handleKeyboardInput(const Uint8* keyboardState, std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject);
    void renderHierarchyTree(const std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject, const Uint8* keyboardState);
    void handleObjectSelection(GameObject* obj, const std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject, const Uint8* keyboardState);
    void deleteSelectedObjects(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject);
    void deleteObjectAndChildren(GameObject* obj, std::vector<GameObject*>& gameObjects);
    void processNewObjects(std::vector<GameObject*>& gameObjects);
    std::vector<GameObject*> getNewObjects(const std::vector<GameObject*>& currentObjects);
    void setupInitialHierarchy(std::vector<GameObject*>& gameObjects);
    void handleParenting(std::vector<GameObject*>& selectedObjects);
    void processParenting(std::vector<GameObject*>& selectedObjects);
    void applyTransforms(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects);
    void renderErrorPopup();
};

#endif // HIERARCHY_WINDOW_H