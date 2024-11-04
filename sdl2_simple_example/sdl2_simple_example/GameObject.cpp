#include "GameObject.h"
#include "Importer.h"
#include <iostream>
#include <string>

extern Importer importer; // Declare extern importer if already created 


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
    //// Crea el objeto nuevo usando el constructor correcto
    //GameObject* newObject = new GameObject(primitiveType, meshData, textureID); // Aseg�rate de que este constructor exista
    //gameObjects.push_back(newObject);

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
