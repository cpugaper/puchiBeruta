#ifndef HIERARCHYWINDOW_H

#include "GameObject.h"
#include <vector>
#include <imgui.h>

class HierarchyWindow {
public:
    void render(std::vector<GameObject*>& gameObjects, GameObject*& selectedObject);
    void handleParenting(); 
    void applyTransforms(std::vector<GameObject*>& gameObjects);
};

#endif // HIERARCHYWINDOW_H
