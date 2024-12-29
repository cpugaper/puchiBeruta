#ifndef HIERARCHYWINDOW_H

#include "GameObject.h"
#include <vector>
#include <imgui.h>

class HierarchyWindow {
public:
    void deleteSelectedObjects(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject);
    void deleteObjectAndChildren(GameObject* obj, std::vector<GameObject*>& gameObjects);
    //void showErrorPopup();
    void render(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject);
    void handleParenting(std::vector<GameObject*>& selectedObjects);
    void applyTransforms(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects);

    void setupInitialHierarchy(std::vector<GameObject*>& gameObjects);
    std::vector<GameObject*> getNewObjects(const std::vector<GameObject*>& currentObjects);

    bool pKeyPressed = false; 
    bool deleteKeyPressed = false;
    std::vector<GameObject*> lastKnownObjects;
};

#endif // HIERARCHYWINDOW_H
