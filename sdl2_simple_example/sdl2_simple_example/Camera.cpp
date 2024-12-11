#include "Camera.h"
#include <GL/glew.h>
#include <iostream>
#include "Variables.h"
#include "ConsoleWindow.h"
#include "MyWindow.h"
#include "Ray.h"

Camera camera;

// Initializes variables related to the position and movement of the camera
Camera::Camera() : position(0.0f, -1.0f, -10.0f), angleX(0.0f), angleY(0.0f), objectAngleX(0.0f), objectAngleY(0.0f), speed(0.1f), altPressed(false), shiftPressed(false), isLeftMouseDragging(false), isRightMouseDragging(false) {}

// Resets the camera's position and orientation based on the selected object
void Camera::reset() {
    if (variables->window->selectedObjects.size() == 1) {
        GameObject* obj = variables->window->selectedObjects.front();
        position = -obj->getPosition();
        position.y -= 1;
        position.z -= 10;
        angleX = angleY = 0.0f;
        objectAngleX = objectAngleY = 0.0f;
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

        objectAngleY += deltaX * 0.1f;
        objectAngleX += deltaY * 0.1f;
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
    if (isRightMouseDragging) {
        if (keyboardState[SDL_SCANCODE_W]) position.z += speed;
        if (keyboardState[SDL_SCANCODE_S]) position.z -= speed;
        if (keyboardState[SDL_SCANCODE_A]) position.x += speed;
        if (keyboardState[SDL_SCANCODE_D]) position.x -= speed;
    }
}

void Camera::applyCameraTransformations() {
    glRotatef(angleY, 1.0f, 0.0f, 0.0f);
    glRotatef(angleX, 0.0f, 1.0f, 0.0f);
    glTranslatef(position.x, position.y, position.z);
    glRotatef(objectAngleX, 1.0f, 0.0f, 0.0f);
    glRotatef(objectAngleY, 0.0f, 1.0f, 0.0f);
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + getForwardDirection(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
}

glm::vec3 Camera::getForwardDirection() const {
    // La direcci�n hacia adelante se calcula como un vector usando los �ngulos de rotaci�n de la c�mara.
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f)); // rotaci�n alrededor del eje Y
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f)); // rotaci�n alrededor del eje X

    // Direcci�n hacia adelante (inicialmente [0.0, 0.0, -1.0], es decir, mirando hacia el eje Z negativo)
    glm::vec4 forward = rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

    return glm::vec3(forward);
}

Ray Camera::getMouseRay(float mouseX, float mouseY, float width, float height) {
    glm::mat4 projection = getProjectionMatrix(width / height);
    glm::mat4 view = getViewMatrix();
    
    // Normalizamos las coordenadas del mouse a valores de [-1, 1]
    glm::vec4 mouseClipSpace = glm::vec4(
        (mouseX / width) * 2.0f - 1.0f,
        1.0f - (mouseY / height) * 2.0f,
        -1.0f, 1.0f
    );

    // Invertimos las matrices de vista y proyecci�n
    glm::mat4 invProjection = glm::inverse(projection);
    glm::mat4 invView = glm::inverse(view);
    
    // Convertimos las coordenadas de la pantalla a espacio de mundo
    glm::vec4 worldSpaceRay = invProjection * mouseClipSpace;
    worldSpaceRay = invView * glm::vec4(worldSpaceRay.x, worldSpaceRay.y, -1.0f, 0.0f);

    // El rayo comienza en la c�mara y tiene una direcci�n que apunta hacia el objeto
    glm::vec3 rayOrigin = position;
    glm::vec3 rayDirection = glm::normalize(glm::vec3(worldSpaceRay));

    console.addLog("gettingmouseray - rayorigin: x = " + std::to_string(rayOrigin.x) + ", y = " + std::to_string(rayOrigin.y) + ", z = " + std::to_string(rayOrigin.z)); 

    return Ray(rayOrigin, rayDirection);
}