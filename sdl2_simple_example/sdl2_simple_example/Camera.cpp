#include "Camera.h"
#include <GL/glew.h>
#include <iostream>
#include "Variables.h"
#include "ConsoleWindow.h"
#include "MyWindow.h"
#include "Ray.h"
#include "SceneWindow.h"

Camera camera;
extern SceneWindow sceneWindow;

// Initializes variables related to the position and movement of the camera
Camera::Camera() : position(0.0f, -1.0f, -10.0f), angleX(0.0f), angleY(0.0f), speed(0.1f), altPressed(false), shiftPressed(false), isLeftMouseDragging(false), isRightMouseDragging(false) {}

// Resets the camera's position and orientation based on the selected object
void Camera::reset() {
    if (variables->window->selectedObjects.size() == 1) {
        GameObject* obj = variables->window->selectedObjects.front();
        position = -obj->getPosition();
        position.y -= 1;
        position.z -= 10;
        angleX = angleY = 0.0f;
        //objectAngleX = objectAngleY = 0.0f;
        speed = 0.1f;
        console.addLog(("Camera reset to object: " + obj->getName()).c_str());
    }
    else if (variables->window->selectedObjects.empty()) {
        console.addLog("No object selected, cannot reset camera");
    }
    else {
        console.addLog("More than 1 object selected, cannot reset camera");
    }
}

void Camera::processKeyDown(const SDL_Keysym& keysym) {
    if (keysym.sym == SDLK_LSHIFT || keysym.sym == SDLK_RSHIFT) {
        shiftPressed = true;
        speed = 0.2f;
    }
    if (keysym.sym == SDLK_LALT || keysym.sym == SDLK_RALT) {
        altPressed = true;
    }
    if (keysym.sym == SDLK_f) {
        console.addLog("Reset key (F) pressed");
        reset();
    }
}

void Camera::processKeyUp(const SDL_Keysym& keysym) {
    if (keysym.sym == SDLK_LSHIFT || keysym.sym == SDLK_RSHIFT) {
        shiftPressed = false;
        speed = 0.1f;
    }
    if (keysym.sym == SDLK_LALT || keysym.sym == SDLK_RALT) {
        altPressed = false;
    }
}

void Camera::processMouseMotion(const SDL_MouseMotionEvent& motion) {
    if (isLeftMouseDragging && altPressed) {
        int deltaX = motion.x - lastMouseX;
        int deltaY = motion.y - lastMouseY;

        angleX += deltaX * 0.1f;
        angleY += deltaY * 0.1f;
        lastMouseX = motion.x;
        lastMouseY = motion.y;
    }
    if (isRightMouseDragging) {
        angleX += motion.xrel * 0.05f;
        angleY += motion.yrel * 0.05f;

        if (angleY > 89.0f) angleY = 89.0f;
        if (angleY < -89.0f) angleY = -89.0f;
    }
}

void Camera::processMouseButtonDown(const SDL_MouseButtonEvent& button) {

    if (button.button == SDL_BUTTON_LEFT && altPressed) {
        isLeftMouseDragging = true;
        lastMouseX = button.x;
        lastMouseY = button.y;
    }
    if (button.button == SDL_BUTTON_RIGHT) {
        isRightMouseDragging = true;
    }
    if (button.button == SDL_BUTTON_LEFT) {
        sceneWindow.checkRaycast(button.x, button.y, variables->window->width(), variables->window->height());// variables->windowWidth, variables->windowHeight);//screenWidth, screenHeight);
    }
}

void Camera::processMouseButtonUp(const SDL_MouseButtonEvent& button) {
    if (button.button == SDL_BUTTON_LEFT) {
        isLeftMouseDragging = false;
    }
    if (button.button == SDL_BUTTON_RIGHT) {
        isRightMouseDragging = false;
    }
}

void Camera::processMouseWheel(const SDL_MouseWheelEvent& wheel) {
    position.z += wheel.y * 0.3f;
}

// Moves the camera based on the keys pressed
void Camera::move(const Uint8* keyboardState) {
    /*if (isRightMouseDragging) {
        if (keyboardState[SDL_SCANCODE_W]) position.z += speed;
        if (keyboardState[SDL_SCANCODE_S]) position.z -= speed;
        if (keyboardState[SDL_SCANCODE_A]) position.x += speed;
        if (keyboardState[SDL_SCANCODE_D]) position.x -= speed;
        if (keyboardState[SDL_SCANCODE_Q]) position.y -= speed;
        if (keyboardState[SDL_SCANCODE_E]) position.y += speed;
    }*/
    if (isRightMouseDragging) {
        glm::vec3 forward = getForwardVector();
        glm::vec3 right = getRightVector();
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        // Calcular el movimiento basado en la dirección de la cámara
        if (keyboardState[SDL_SCANCODE_W])
            position -= forward * speed;
        if (keyboardState[SDL_SCANCODE_S])
            position += forward * speed;
        if (keyboardState[SDL_SCANCODE_A])
            position += right * speed;
        if (keyboardState[SDL_SCANCODE_D])
            position -= right * speed;
        if (keyboardState[SDL_SCANCODE_Q])
            position -= up * speed;
        if (keyboardState[SDL_SCANCODE_E])
            position += up * speed;
    }
}

void Camera::applyCameraTransformations() {
    glRotatef(angleY, 1.0f, 0.0f, 0.0f);
    glRotatef(angleX, 0.0f, 1.0f, 0.0f);
    glTranslatef(position.x, position.y, position.z);
    //glRotatef(objectAngleX, 1.0f, 0.0f, 0.0f);
    //glRotatef(objectAngleY, 0.0f, 1.0f, 0.0f);
}

glm::vec3 Camera::getForwardVector() {
    // Convertir ángulos a radianes
    float angleXRad = glm::radians(angleX);
    float angleYRad = glm::radians(angleY);

    // Calcular el vector forward basado en los ángulos de rotación
    return glm::vec3(
        sin(angleXRad) * cos(angleYRad),
        -sin(angleYRad),
        -cos(angleXRad) * cos(angleYRad)
    );
}

glm::vec3 Camera::getRightVector() {
    // El vector right es perpendicular al forward y al up (0,1,0)
    glm::vec3 forward = getForwardVector();
    return glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
}

bool Camera::isInFrustum(glm::vec3 corners[8]) {
    for (int i = 0; i < 8; ++i) {
        const glm::vec3& vertex = corners[i];
        glm::vec4 clipSpaceVertex = sceneWindow.ProjectionMatrix() * sceneWindow.ViewMatrix() * glm::vec4(vertex, 1.0f);

        glm::vec3 ndcVertex = glm::vec3(clipSpaceVertex) / clipSpaceVertex.w;

        bool inFrustum = ndcVertex.x >= -1.0f && ndcVertex.x <= 1.0f &&
            ndcVertex.y >= -1.0f && ndcVertex.y <= 1.0f &&
            ndcVertex.z >= -1.0f && ndcVertex.z <= 1.0f;

        if (inFrustum) {
            return true;  
        }
    }

    return false;
}
