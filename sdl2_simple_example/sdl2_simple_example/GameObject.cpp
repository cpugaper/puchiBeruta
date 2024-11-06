#include "GameObject.h"
#include "Importer.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <random>

extern Importer importer; // Declare extern importer if already created 

GameObject::GameObject(const std::string& name, const MeshData& mesh, GLuint texID)
    : name(name), meshData(mesh), textureID(texID), position(0.0f), rotation(0.0f), scale(1.0f), uuid(GenerateUUID()) {
    std::cout << "GameObject created with UUID: " << uuid << std::endl;
}

std::string GameObject::GenerateUUID() {
    std::stringstream ss;
    for (int i = 0; i < 16; i++) {
        int byte = rand() % 256;
        ss << std::hex << std::setw(2) << std::setfill('0') << byte;
    }
    return ss.str();
}

void GameObject::addChild(const GameObject& child) {
    children.push_back(child);
}

void GameObject::createPrimitive(const std::string& primitiveType, std::vector<GameObject*>& gameObjects) {
    MeshData meshData;
    GLuint textureID = 0;
    std::string filePath = "Assets\\Primitives\\";

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
            //gameObjects.emplace_back(primitiveType, meshData, textureID);
            std::cout << "Model " << primitiveType << " loaded from " << filePath << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error when loading model " << primitiveType << ": " << e.what() << std::endl;
    }
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

void GameObject::setPosition(const glm::vec3& newPosition) {
    position = newPosition;
}

void GameObject::setRotation(const glm::vec3& newRotation) {
    rotation = newRotation;
}

void GameObject::setScale(const glm::vec3& newScale) {
    scale = newScale;
}

template <class Archive>
void GameObject::serialize(Archive& archive) {
    archive(CEREAL_NVP(uuid), CEREAL_NVP(name), CEREAL_NVP(position), CEREAL_NVP(rotation), CEREAL_NVP(scale), CEREAL_NVP(children));
}

