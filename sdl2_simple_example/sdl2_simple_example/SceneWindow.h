#ifndef SCENEWINDOW_H

#include "GameObject.h"
#include "Renderer.h"
#include <vector>
#include "imgui.h"
#include "Ray.h"

class SceneWindow {
public:
    bool isActive;
    void render();
    void updateSceneSize();

    Ray getRayFromMouse(int mouseX, int mouseY, int screenWidth, int screenHeight);
    void checkRaycast(int mouseX, int mouseY, int screenWidth, int screenHeight);
    void DrawRay(const Ray& ray, float length);

    ImVec2 windowSize;
    ImVec2 windowPos;
    ImVec2 contentPos;
    ImVec2 contentRegionAvail;

    Ray* rayo;
    bool rayoexists;
};

#endif // SCENEWINDOW_H
