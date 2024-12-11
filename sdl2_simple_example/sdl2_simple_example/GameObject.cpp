#include "GameObject.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <random>
#include <fstream>
#include <cereal/archives/json.hpp>
#include "Importer.h"
#include "ConsoleWindow.h"

extern Importer importer;
std::vector<GameObject> gameObjects;

GameObject::GameObject(const std::string& name, const MeshData& mesh, GLuint texID)
    : name(name), meshData(mesh), textureID(texID), position(0.0f), rotation(0.0f), scale(1.0f), uuid(GenerateUUID()) {
    initialPosition = position;
    initialRotation = rotation;
    initialScale = scale;
    console.addLog("GameObject created with UUID: " + uuid);
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

// Adds a child object to the list of children of this object
void GameObject::addChild(GameObject* child) {
    child->parent = this;
    children.push_back(child);
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
            console.addLog("Model " + primitiveType + " loaded from " + filePath);
        }
    }
    catch (const std::exception& e) {
        console.addLog("Error when loading model " + primitiveType + ": " + e.what());
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

// Transforms for parenting logic

glm::mat4 GameObject::getFinalTransformMatrix() const {
    glm::mat4 finalTransform = getTransformMatrix();

    if (parent != nullptr) {
        finalTransform = parent->getFinalTransformMatrix() * finalTransform;
    }
    return finalTransform;
}

//void GameObject::setTransformFromMatrix(const glm::mat4& transform) {
//    scale = glm::vec3(
//        glm::length(glm::vec3(transform[0])),
//        glm::length(glm::vec3(transform[1])),
//        glm::length(glm::vec3(transform[2]))
//    );
//
//    position = glm::vec3(transform[3]);
//
//    glm::mat3 rotationMatrix = glm::mat3(
//        glm::vec3(transform[0]) / scale.x,
//        glm::vec3(transform[1]) / scale.y,
//        glm::vec3(transform[2]) / scale.z
//    );
//    rotation = glm::degrees(glm::eulerAngles(glm::quat_cast(rotationMatrix)));
//}


void GameObject::updateChildTransforms() {

    globalTransform = getFinalTransformMatrix();

    for (GameObject* child : children) {
        child->position = position + child->initialPosition;
        child->rotation = rotation + child->initialRotation;
        child->scale = scale * child->initialScale;

        child->updateChildTransforms();

   /*     glm::mat4 parentGlobalTransform = globalTransform;
        glm::mat4 childGlobalTransform = child->getFinalTransformMatrix();
        glm::mat4 newLocalTransform = glm::inverse(parentGlobalTransform) * childGlobalTransform;

        child->setTransformFromMatrix(newLocalTransform);
        child->updateChildTransforms();*/
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

bool GameObject::intersectsRay(const Ray& ray, float& t) {
    // AABB de este objeto
    glm::vec3 min = position - (scale / 2.0f);
    glm::vec3 max = position + (scale / 2.0f);

    float tmin = (min.x - ray.origin.x) / ray.direction.x;
    float tmax = (max.x - ray.origin.x) / ray.direction.x;

    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (min.y - ray.origin.y) / ray.direction.y;
    float tymax = (max.y - ray.origin.y) / ray.direction.y;

    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax)) return false;

    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    float tzmin = (min.z - ray.origin.z) / ray.direction.z;
    float tzmax = (max.z - ray.origin.z) / ray.direction.z;

    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax)) return false;

    t = tmin; // El valor de t donde se produce la intersección
    return true;
}