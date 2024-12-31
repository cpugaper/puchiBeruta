#ifndef INSPECTORWINDOW_H

#include "GameObject.h"
#include <vector>
#include <imgui.h>

class InspectorWindow {
public:
    void render(GameObject* selectedObject);
};

#endif // INSPECTORWINDOW_H