#include "GameObject.h"
#include "Importer.h"
#include <iostream>
#include <string>

extern Importer importer; // Declare extern importer if already created 

void GameObject::createPrimitive(const std::string& primitiveType, std::vector<GameObject>& gameObjects) {
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
            gameObjects.emplace_back(primitiveType, meshData, textureID);
            std::cout << "Model " << primitiveType << " loaded from " << filePath << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error when loading model " << primitiveType << ": " << e.what() << std::endl;
    }
}
