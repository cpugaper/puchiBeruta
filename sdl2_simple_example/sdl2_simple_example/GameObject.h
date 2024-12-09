#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>
#include <GL/glew.h>

struct MeshData {
    std::string name;
    std::vector<GLfloat> vertices;
    std::vector<uint32_t> indices;
    std::vector<GLfloat> textCoords;
    std::vector<GLfloat> normals;
    glm::mat4 transform;

  /*  template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(name), CEREAL_NVP(vertices), CEREAL_NVP(indices), CEREAL_NVP(textCoords), CEREAL_NVP(transform));
    }*/
};

// To be able to serialize glm::vec3 & glm::mat4
//namespace glm {
//    template <class Archive>
//    void serialize(Archive& archive, glm::vec3& vec) {
//        archive(CEREAL_NVP(vec.x), CEREAL_NVP(vec.y), CEREAL_NVP(vec.z));
//    }
//
//    template <class Archive>
//    void serialize(Archive& archive, glm::mat4& mat) {
//        for (int i = 0; i < 4; ++i) {
//            for (int j = 0; j < 4; ++j) {
//                archive(CEREAL_NVP(mat[i][j]));
//            }
//        }
//    }
//}

class GameObject {
public:
    static std::vector<GameObject*> gameObjects;

    std::string uuid;
    std::string name;
    std::vector<GameObject*> children;
    GameObject* parent = nullptr; 
    MeshData meshData;
    GLuint textureID;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    glm::vec3 initialPosition;
    glm::vec3 initialRotation;
    glm::vec3 initialScale;

    glm::mat4 globalTransform;

    GameObject(const std::string& name, const MeshData& mesh, GLuint texID);

    // PARENTING
    void addChild(GameObject* child);
    void removeChild(GameObject* child);
    void updateChildTransforms();
    glm::mat4 getFinalTransformMatrix() const;

    static void createPrimitive(const std::string& primitiveType, std::vector<GameObject*>& gameObjects);
    glm::mat4 getTransformMatrix() const;

    const std::string& getName() const { return name; }
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getRotation() const { return rotation; }
    glm::vec3 getScale() const { return scale; }

    void setPosition(const glm::vec3& newPosition);
    void setRotation(const glm::vec3& newRotation);
    void setScale(const glm::vec3& newScale);

    void resetTransform();

   /* MeshData toMeshData() const {
        MeshData meshData;
        meshData.name = this->name;
        meshData.vertices = this->meshData.vertices;  
        meshData.indices = this->meshData.indices;   
        meshData.textCoords = this->meshData.textCoords;
        meshData.transform = getTransformMatrix();    
        return meshData;
    }*/

    //template <class Archive>
    //void serialize(Archive& archive) {
    //    uint32_t texID = static_cast<uint32_t>(textureID);
    //    archive(CEREAL_NVP(uuid), CEREAL_NVP(name), CEREAL_NVP(children), CEREAL_NVP(meshData),
    //        CEREAL_NVP(textureID), CEREAL_NVP(position), CEREAL_NVP(rotation), CEREAL_NVP(scale));
    //}

    static std::string GenerateUUID();

    MeshData* getMeshData() { return &meshData; } 
    void setMeshData(const MeshData& data) { meshData = data; }
};

#endif // GAMEOBJECT_H