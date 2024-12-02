#ifndef SCENEWINDOW_H

#include "GameObject.h"
#include "Renderer.h"
#include <vector>
#include "imgui.h"

class SceneWindow {
public:
    void render();
    void updateSceneSize();
};

#endif // SCENEWINDOW_H
