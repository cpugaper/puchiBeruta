#ifndef HIERARCHYWINDOW_H

#include "GameObject.h"
#include <vector>
#include <imgui.h>

class HierarchyWindow {
public:
    void render(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects, GameObject*& selectedObject);
    void handleParenting(std::vector<GameObject*>& selectedObjects);
    void applyTransforms(std::vector<GameObject*>& gameObjects, std::vector<GameObject*>& selectedObjects);
};

#endif // HIERARCHYWINDOW_H
