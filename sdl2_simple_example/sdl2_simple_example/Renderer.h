#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <string>
#include "GameObject.h"
#include "Camera.h"

extern GLuint framebuffer;
extern GLuint textureColorbuffer;
extern GLuint rbo;

extern int framebufferWidth;
extern int framebufferHeight;

class Renderer {
public:
	static Renderer renderer;

	void initOpenGL();
	bool processEvents(Camera& camera, std::vector<GameObject>& gameObjects, const char*& fbxFile);
	void HandleDroppedFile(const char* droppedFile);
	void HandleDragDropTarget();
	void drawGrid(float spacing);
	void render(const std::vector<GameObject*>& gameObjects);
	std::string getFileName(const std::string& path);
	void createFrameBuffer(int width, int height);
	void cleanupFrameBuffer();
};

#endif // RENDERER_H