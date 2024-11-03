#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Importer.h"

class GameObject {
public:
    std::string name;
    std::vector<GameObject> children;
    MeshData meshData;
    GLuint textureID;

    // Object Transformation
    glm::vec3 position;
    glm::vec3 rotation; 
    glm::vec3 scale;

    GameObject(const std::string& name, const MeshData& mesh, GLuint texID) : name(name), meshData(mesh), textureID(texID), position(0.0f), rotation(0.0f), scale(1.0f) {}

    void addChild(const GameObject& child) {
        children.push_back(child);
    }

    static void createPrimitive(const std::string& primitiveType, std::vector<GameObject>& gameObjects);

    glm::mat4 getTransformMatrix() const {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, position);
        transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1, 0, 0)); 
        transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0, 1, 0)); 
        transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0, 0, 1)); 
        transform = glm::scale(transform, scale);
        return transform;
    }

    glm::vec3 getPosition() const { return position; }
    glm::vec3 getRotation() const { return rotation; }
    glm::vec3 getScale() const { return scale; }
};

#endif // GAMEOBJECT_H