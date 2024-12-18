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
    if (button.button == SDL_BUTTON_LEFT) {
        checkRaycast(button.x, button.y, variables->window->width(), variables->window->height());// variables->windowWidth, variables->windowHeight);//screenWidth, screenHeight);
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

Ray Camera::getRayFromMouse(int mouseX, int mouseY, int screenWidth, int screenHeight) {
    console.addLog("Entra funcion getrayfrommouse");
    console.addLog("X: " + mouseX); 
    console.addLog("Y: " + mouseY);
    // Convertir las coordenadas del mouse a coordenadas normalizadas (NDC)
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;  // Invertimos el eje Y

    // Obtener la matriz de proyección inversa y vista inversa
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 inverseProjectionView = glm::inverse(projection * view);

    // Proyectar el punto del ratón hacia un rayo en el espacio 3D
    glm::vec4 clipSpacePos(x, y, -1.0f, 1.0f);
    glm::vec4 worldPos = inverseProjectionView * clipSpacePos;

    glm::vec3 rayDirection = glm::normalize(glm::vec3(worldPos) - position);
    return Ray(position, rayDirection);
}
// Método que verifica si el rayo interseca con algún objeto en la escena
void Camera::checkRaycast(int mouseX, int mouseY, int screenWidth, int screenHeight) {
    Ray ray = getRayFromMouse(mouseX, mouseY, screenWidth, screenHeight);

    for (auto& obj : variables->window->gameObjects) {
        MeshData* meshData = obj->getMeshData();
        if (meshData) {
            for (size_t i = 0; i < meshData->indices.size(); i += 3) {
                glm::vec3 vertex1 = glm::vec3(meshData->vertices[meshData->indices[i] * 3], meshData->vertices[meshData->indices[i] * 3 + 1], meshData->vertices[meshData->indices[i] * 3 + 2]);
                glm::vec3 vertex2 = glm::vec3(meshData->vertices[meshData->indices[i + 1] * 3], meshData->vertices[meshData->indices[i + 1] * 3 + 1], meshData->vertices[meshData->indices[i + 1] * 3 + 2]);
                glm::vec3 vertex3 = glm::vec3(meshData->vertices[meshData->indices[i + 2] * 3], meshData->vertices[meshData->indices[i + 2] * 3 + 1], meshData->vertices[meshData->indices[i + 2] * 3 + 2]);
                
                glm::vec3 edge1 = vertex2 - vertex1;
                glm::vec3 edge2 = vertex3 - vertex1;
                glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

               /* ImGui::Text("Triangle %d Normal: %.3f, %.3f, %.3f", i / 3, faceNormal.x, faceNormal.y, faceNormal.z);

                std::string normalKey = std::to_string(faceNormal.x) + "," + std::to_string(faceNormal.y) + "," + std::to_string(faceNormal.z);
*/

                float t = 0.0f;
                if (ray.intersectsTriangle(vertex1, vertex2, vertex3, t)) {
                    console.addLog("Objeto seleccionado: " + obj->getName());
                    console.addLog("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA ");
                    variables->window->selectedObjects.push_back(obj);
                    break;
                }
            }
        }
    }
}