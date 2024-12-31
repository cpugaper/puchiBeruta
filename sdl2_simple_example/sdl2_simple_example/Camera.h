#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <SDL2/SDL_events.h>
#include <cereal/types/vector.hpp>
#include <cereal/types/base_class.hpp> 
#include <cereal/archives/json.hpp>

class Camera {
public:
	static Camera camera;

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
	glm::vec3 getForwardVector();
	glm::vec3 getRightVector();

	bool isInFrustum(glm::vec3 corners[8]);

	glm::vec3 position; 
	float angleX, angleY;

	glm::vec3 initPosition;
	float initAngleX;
	float initAngleY;

private:

	float speed;
	bool shiftPressed, altPressed, isLeftMouseDragging, isRightMouseDragging;
	int lastMouseX, lastMouseY;

	glm::vec4 leftPlaneFrustrum;
	glm::vec4 rightPlaneFrustrum;
	glm::vec4 bottomPlaneFrustrum;
	glm::vec4 topPlaneFrustrum;
	glm::vec4 nearPlaneFrustrum;
	glm::vec4 farPlaneFrustrum;
};

#endif // CAMERA_H