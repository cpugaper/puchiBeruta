#include "GameObject.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <random>
#include <fstream>
#include "Importer.h"
#include "ConsoleWindow.h"

extern Importer importer;
std::vector<GameObject> gameObjects;

GameObject::GameObject(const std::string& name, const MeshData& mesh, GLuint texID, const std::string& texPath)
    : name(name), meshData(mesh), textureID(texID), texturePath(texPath), position(0.0f), rotation(0.0f), scale(1.0f), uuid(GenerateUUID()) {
    initialPosition = position;
    initialRotation = rotation;
    initialScale = scale;
    console.addLog("GameObject created with UUID: " + uuid);
}

void GameObject::updateMovement(float deltaTime) {
    if (movementState == MovementState::Running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float elapsedTime = std::chrono::duration<float>(currentTime - startTime).count();
        float angle = startAngle + angularSpeed * elapsedTime;

        position.x = radius * cos(angle);
        position.z = radius * sin(angle);

        startTime = currentTime;
    }
}

void GameObject::startMovement() {
    if (movementState == MovementState::Stopped || movementState == MovementState::Paused) {
        movementState = MovementState::Running;
        startTime = std::chrono::high_resolution_clock::now();  
        startAngle = atan2(position.z, position.x);  
    }
}

void GameObject::pauseMovement() {
    if (movementState == MovementState::Running) {
        movementState = MovementState::Paused;
    }
}

void GameObject::stopMovement() {
    movementState = MovementState::Stopped;
    resetTransform();
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
    console.addLog("Empty object created");
}

void GameObject::createDynamicObject(const std::string& name, std::vector<GameObject*>& gameObjects) {
    createPrimitive("Sphere", gameObjects);

    GameObject* dynamicObject = gameObjects.back();
    dynamicObject->name = name;

    dynamicObject->movementState = MovementState::Stopped; 
    dynamicObject->angularSpeed = 0.0f;  
    dynamicObject->radius = 0.0f; 
    dynamicObject->startAngle = 0.0f;  
    dynamicObject->startTime = std::chrono::high_resolution_clock::now(); 

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
    glm::vec3 localPosition = position;
    glm::vec3 localRotation = rotation;
    glm::vec3 localScale = scale;

    globalTransform = getFinalTransformMatrix();

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
}