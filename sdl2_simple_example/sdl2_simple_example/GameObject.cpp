#include "GameObject.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <random>
#include <fstream>
#include "Importer.h"
#include "ConsoleWindow.h"
#include "SimulationManager.h"

extern Importer importer;
std::vector<GameObject> gameObjects;

GameObject::GameObject(const std::string& name, const MeshData& mesh, GLuint texID, const std::string& texPath)
    : name(name), meshData(mesh), textureID(texID), texturePath(texPath), position(0.0f), rotation(0.0f), scale(1.0f), uuid(GenerateUUID()), elapsedPausedTime(0.0f) {
    initialPosition = position;
    initialRotation = rotation;
    initialScale = scale;
    console.addLog("GameObject created with UUID: " + uuid);
}

void GameObject::updateMovement(float deltaTime) {
    if (movementState == MovementState::Running) {
        position.x += movementDirection * speed * deltaTime;
        if (position.x >= movementRange.second || position.x <= movementRange.first) {
            movementDirection *= -1; 
        }
    }
}

void GameObject::startMovement() {
    if (movementState != MovementState::Running) {
        movementState = MovementState::Running;
        startTime = std::chrono::high_resolution_clock::now() - std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::duration<float>(elapsedPausedTime));
        console.addLog(name + " started moving.");
    }
}

void GameObject::pauseMovement() {
    if (movementState == MovementState::Running) {
        movementState = MovementState::Paused;
        auto currentTime = std::chrono::high_resolution_clock::now();
        elapsedPausedTime = std::chrono::duration<float>(currentTime - startTime).count();
        console.addLog(name + " paused movement.");
    }
}

void GameObject::stopMovement() {
    if (movementState != MovementState::Stopped) {
        movementState = MovementState::Stopped;
        elapsedPausedTime = 0.0f; 
        resetTransform(); 
        console.addLog(name + " stopped and reset position.");
    }
}


// Generate a random UUID to identify each object uniquely
std::string GameObject::GenerateUUID() {
    std::stringstream ss;
    for (int i = 0; i < 16; i++) {
        int byte = rand() % 256;
        ss << std::hex << std::setw(2) << std::setfill('0') << byte;
    }
    return ss.str();
}

void GameObject::setTexture(const std::string& path, GLuint texID) {
    texturePath = path;
    textureID = texID;
}

void GameObject::loadTextureFromPath() {
    if (!texturePath.empty()) {
        textureID = importer.loadTexture(texturePath);
        if (textureID == 0) {
            console.addLog("Failed to load texture from: " + texturePath);
        }
    }
    else {
        console.addLog("Texture path is empty for GameObject: " + name);
    }
}

// Adds a child object to the list of children of this object
void GameObject::addChild(GameObject* child) {
    child->parent = this;

    child->initialPosition = child->position - this->position;  
    child->initialRotation = child->rotation - this->rotation; 
    child->initialScale = child->scale / this->scale;

    children.push_back(child);

    updateChildTransforms();
}

void GameObject::removeChild(GameObject* child) {
    auto it = std::find(children.begin(), children.end(), child);

    if (it != children.end()) {
        children.erase(it);
        child->parent = nullptr;
    }
}

void GameObject::createPrimitive(const std::string& primitiveType, std::vector<GameObject*>& gameObjects) {
    MeshData meshData;
    GLuint textureID = 0;
    std::string filePath = "sdl2_simple_example/Assets/Primitives/";

    if (primitiveType == "Sphere") {
        filePath += "sphere.fbx";
    }
    else if (primitiveType == "Cube") {
        filePath += "cube.fbx";
    }
    else if (primitiveType == "Cylinder") {
        filePath += "cylinder.fbx";
    }
    else if (primitiveType == "Cone") {
        filePath += "cone.fbx";
    }
    else if (primitiveType == "Torus") {
        filePath += "torus.fbx";
    }
    else if (primitiveType == "Plane") {
        filePath += "plane.fbx";
    }

    // Load model with Assimp, using importer
    try {
        std::vector<MeshData> meshes = importer.loadFBX(filePath, textureID);
        if (!meshes.empty()) {
            meshData = meshes[0];
            GameObject* modelObject = new GameObject(primitiveType, meshData, textureID);
            gameObjects.push_back(modelObject);
            SimulationManager::simulationManager.trackObject(modelObject);
            console.addLog("Model " + primitiveType + " loaded from " + filePath);
        }
    }
    catch (const std::exception& e) {
        console.addLog("Error when loading model " + primitiveType + ": " + e.what());
    }
}

void GameObject::createEmptyObject(const std::string& name, std::vector<GameObject*>& gameObjects) {
    MeshData emptyMeshData;
    GLuint emptyTextureID = 0;
    GameObject* emptyObject = new GameObject(name, emptyMeshData, emptyTextureID);

    gameObjects.push_back(emptyObject);

    SimulationManager::simulationManager.trackObject(emptyObject);

    console.addLog("Empty object created");
}

void GameObject::createDynamicObject(const std::string& name, std::vector<GameObject*>& gameObjects) {
    createPrimitive("Sphere", gameObjects);

    GameObject* dynamicObject = gameObjects.back();
    dynamicObject->name = name;

    dynamicObject->movementState = MovementState::Stopped; 
    dynamicObject->speed = 3.0f;  
    dynamicObject->movementRange = std::make_pair(-5.0f, 5.0f);  
    dynamicObject->movementDirection = 1.0f; 
    dynamicObject->startTime = std::chrono::high_resolution_clock::now();

    SimulationManager::simulationManager.trackObject(dynamicObject);

    console.addLog("Dynamic object created: " + name);
}


glm::mat4 GameObject::getTransformMatrix() const {
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0, 0, 1));
    transform = glm::scale(transform, scale);
    return transform;
}

// Transforms for parenting logic

glm::mat4 GameObject::getFinalTransformMatrix() const {
    glm::mat4 localTransform = getTransformMatrix();

    if (parent != nullptr) {
        return parent->getFinalTransformMatrix() * localTransform;
    }
    return localTransform;
}

void GameObject::updateChildTransforms() {
    for (GameObject* child : children) {
        child->position = this->position + child->initialPosition;
        child->rotation = this->rotation + child->initialRotation;
        child->scale = this->scale * child->initialScale;

        child->updateChildTransforms();
    }
}

void GameObject::setPosition(const glm::vec3& newPosition) {
    position = newPosition;
}

void GameObject::setRotation(const glm::vec3& newRotation) {
    rotation = newRotation;
}

void GameObject::setScale(const glm::vec3& newScale) {
    scale = newScale;
}

void GameObject::resetTransform() {
    position = initialPosition;
    rotation = initialRotation;
    scale = initialScale;

    movementDirection = 1.0f;

    if (parent != nullptr) {
        parent->updateChildTransforms();
    }
}