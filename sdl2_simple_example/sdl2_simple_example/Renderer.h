#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <string>
#include "GameObject.h"
#include "Camera.h"

extern GLuint framebuffer;
extern GLuint textureColorbuffer;
extern GLuint rbo;

void initOpenGL();
bool processEvents(Camera& camera, std::vector<GameObject>& gameObjects, const char*& fbxFile);
void render(const std::vector<GameObject*>& gameObjects);
std::string getFileName(const std::string& path);
void createFrameBuffer(int width, int height);
void cleanupFrameBuffer();

#endif // RENDERER_H
