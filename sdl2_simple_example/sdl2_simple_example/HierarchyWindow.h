#ifndef HIERARCHY_WINDOW_H
#define HIERARCHY_WINDOW_H

#include "GameObject.h"
#include <vector>
#include <imgui.h>
#include <SDL2/SDL_events.h>

class HierarchyWindow {
public:
    void render(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject);

    void deleteSelectedObjects(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject);

    void setupInitialHierarchy(std::vector<GameObject*>& gameObjects);

    bool pKeyPressed = false;
    bool deleteKeyPressed = false;
    std::vector<GameObject*> lastKnownObjects;

private:
    void deleteObjectAndChildren(GameObject* obj, std::vector<GameObject*>& gameObjects);

    void renderHierarchyTree(const std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject, const Uint8* keyboardState);
    void renderErrorPopup();

    void handleKeyboardInput(const Uint8* keyboardState, std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject);
    void handleObjectSelection(GameObject* obj, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject, const Uint8* keyboardState);

    void handleParenting(std::vector<GameObject*>& selectedObjects);
    void processParenting(std::vector<GameObject*>& selectedObjects);
    void applyTransforms(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects);

    void processNewObjects(std::vector<GameObject*>& gameObjects);
    std::vector<GameObject*> getNewObjects(const std::vector<GameObject*>& currentObjects);
};

#endif // HIERARCHY_WINDOW_H