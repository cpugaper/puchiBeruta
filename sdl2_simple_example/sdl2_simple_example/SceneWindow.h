#ifndef SCENEWINDOW_H

#include "GameObject.h"
#include "Renderer.h"
#include <vector>
#include "imgui.h"
#include "Ray.h"

class SceneWindow {
public:
    void render();
    void loadIcons();
    void updateSceneSize();

    Ray getRayFromMouse(int mouseX, int mouseY, int screenWidth, int screenHeight);
    void checkRaycast(int mouseX, int mouseY, int screenWidth, int screenHeight);
};

#endif // SCENEWINDOW_H
