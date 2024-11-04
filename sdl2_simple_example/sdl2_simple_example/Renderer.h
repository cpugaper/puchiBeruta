#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <string>
#include "GameObject.h"
#include "Camera.h"

void initOpenGL();
bool processEvents(Camera& camera, std::vector<GameObject>& gameObjects, const char*& fbxFile);
void render(const std::vector<GameObject*>& gameObjects);
std::string getFileName(const std::string& path);

#endif // RENDERER_H
