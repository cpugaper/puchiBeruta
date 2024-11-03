#include "GameObject.h"
#include "Importer.h"
#include <iostream>
#include <string>

extern Importer importer; // Declaramos el importador externo, si ya est� creado en otro lugar

void GameObject::createPrimitive(const std::string& primitiveType, std::vector<GameObject>& gameObjects) {
    MeshData meshData;
    GLuint textureID = 0;
    std::string filePath = "Assets/Primitives/";

    // Seleccionamos el archivo seg�n el tipo de forma
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

    // Cargar el modelo con Assimp usando `importer`
    try {
        std::vector<MeshData> meshes = importer.loadFBX(filePath, textureID);
        if (!meshes.empty()) {
            meshData = meshes[0]; // Suponemos que el modelo tiene una sola malla
            gameObjects.emplace_back(primitiveType, meshData, textureID);
            std::cout << "Modelo " << primitiveType << " cargado desde " << filePath << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error al cargar el modelo de " << primitiveType << ": " << e.what() << std::endl;
    }
}
