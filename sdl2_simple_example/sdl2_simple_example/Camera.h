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

	glm::vec3 position; 
	float angleX, angleY;
	//float objectAngleX, objectAngleY;
private:

	float speed;
	bool shiftPressed, altPressed, isLeftMouseDragging, isRightMouseDragging;
	int lastMouseX, lastMouseY;

	/*template <class Archive>
	void serialize(Archive& archive) {
		archive(CEREAL_NVP(position), CEREAL_NVP(angleX), CEREAL_NVP(angleY), CEREAL_NVP(objectAngleX), CEREAL_NVP(objectAngleY), CEREAL_NVP(speed),
			CEREAL_NVP(shiftPressed), CEREAL_NVP(altPressed), CEREAL_NVP(isLeftMouseDragging), CEREAL_NVP(isRightMouseDragging),
			CEREAL_NVP(lastMouseX), CEREAL_NVP(lastMouseY));
	}*/
};

#endif // CAMERA_H