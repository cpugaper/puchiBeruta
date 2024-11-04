#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <SDL2/SDL_events.h>

class Camera {
public:
	Camera();

	void reset();

	void processMouseMotion(const SDL_MouseMotionEvent& motion);
	void processMouseWheel(const SDL_MouseWheelEvent& wheel);
	void processKeyDown(const SDL_Keysym& keysym);
	void processKeyUp(const SDL_Keysym& keysym);
	void processMouseButtonDown(const SDL_MouseButtonEvent& button);
	void processMouseButtonUp(const SDL_MouseButtonEvent& button);

	void move(const Uint8* keyboardState);

	void applyCameraTransformations();

private:
	glm::vec3 position;
	float angleX, angleY;
	float objectAngleX, objectAngleY;
	float speed;
	bool shiftPressed, altPressed, isLeftMouseDragging, isRightMouseDragging;
	int lastMouseX, lastMouseY;
};

#endif