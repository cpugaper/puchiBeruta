#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Importer.h"
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>

class GameObject {
public:
    std::string uuid;
    std::string name;
    std::vector<GameObject> children;
    MeshData meshData;
    GLuint textureID;

    // Object Transformation
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    GameObject(const std::string& name, const MeshData& mesh, GLuint texID);

    void addChild(const GameObject& child);

    static void createPrimitive(const std::string& primitiveType, std::vector<GameObject*>& gameObjects);

    glm::mat4 getTransformMatrix() const;

    const std::string& getName() const { return name; }
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getRotation() const { return rotation; }
    glm::vec3 getScale() const { return scale; }

    void setPosition(const glm::vec3& newPosition);
    void setRotation(const glm::vec3& newRotation);
    void setScale(const glm::vec3& newScale);

    template <class Archive>
    void serialize(Archive& archive);

    static std::string GenerateUUID();
       
};

#endif // GAMEOBJECT_H