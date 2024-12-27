#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <chrono>
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

    template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(name), CEREAL_NVP(vertices), CEREAL_NVP(indices), CEREAL_NVP(textCoords), CEREAL_NVP(transform));
    }
};

// To be able to serialize glm::vec3 & glm::mat4
namespace glm {
    template <class Archive>
    void serialize(Archive& archive, glm::vec3& vec) {
        archive(CEREAL_NVP(vec.x), CEREAL_NVP(vec.y), CEREAL_NVP(vec.z));
    }

    template <class Archive>
    void serialize(Archive& archive, glm::mat4& mat) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                archive(CEREAL_NVP(mat[i][j]));
            }
        }
    }
}

enum class MovementState {
    Stopped,
    Running,
    Paused
};

class GameObject {
public:
    std::string uuid;
    std::string name;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    MeshData meshData;
    GLuint textureID;
    std::string texturePath;

    glm::vec3 initialPosition;
    glm::vec3 initialRotation;
    glm::vec3 initialScale;

    static std::vector<GameObject*> gameObjects;
    std::vector<std::string> pendingChildUUIDs;
    std::vector<GameObject*> children;
    GameObject* parent = nullptr;

    glm::mat4 globalTransform;

    // Simulation States
    MovementState movementState;
    int movementDirection;
    float speed;
    float elapsedPausedTime = 0.0f;
    std::pair<float, float> movementRange;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

    bool active = true;

    GameObject(const std::string& name, const MeshData& mesh, GLuint texID, const std::string& texPath = "");

    void updateMovement(float deltaTime);
    void startMovement();
    void pauseMovement();
    void stopMovement();

    // PARENTING
    void addChild(GameObject* child);
    void removeChild(GameObject* child);
    void updateChildTransforms();
    glm::mat4 getFinalTransformMatrix() const;

    static void createPrimitive(const std::string& primitiveType, std::vector<GameObject*>& gameObjects);
    static void createEmptyObject(const std::string& name, std::vector<GameObject*>& gameObjects);
    static void createDynamicObject(const std::string& name, std::vector<GameObject*>& gameObjects);
    glm::mat4 getTransformMatrix() const;

    const std::string& getName() const { return name; }
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getRotation() const { return rotation; }
    glm::vec3 getScale() const { return scale; }

    void setPosition(const glm::vec3& newPosition);
    void setRotation(const glm::vec3& newRotation);
    void setScale(const glm::vec3& newScale);
    void resetTransform();

    void setTexture(const std::string& path, GLuint texID);
    void loadTextureFromPath();

    static std::string GenerateUUID();

    MeshData* getMeshData() { return &meshData; }
    void setMeshData(const MeshData& data) { meshData = data; }

    bool getActive() const { return active; }
    void setActive(bool isActive) { active = isActive; }

    template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(uuid), CEREAL_NVP(name), CEREAL_NVP(position), CEREAL_NVP(rotation), CEREAL_NVP(scale), CEREAL_NVP(meshData), CEREAL_NVP(textureID), CEREAL_NVP(texturePath));

        std::vector<std::string> childUUIDs;
        if constexpr (Archive::is_saving::value) {
            childUUIDs.reserve(children.size());
            for (const auto& child : children) {
                childUUIDs.push_back(child->uuid);
            }
        }
        archive(CEREAL_NVP(childUUIDs));

        if constexpr (Archive::is_loading::value) {
            pendingChildUUIDs = childUUIDs;
        }
    }
};

// To work with cereal 
struct GameObjectWrapper {
    GameObject* ptr;
    GameObjectWrapper(GameObject* p = nullptr) : ptr(p) {}
    template <class Archive>
    void serialize(Archive& ar);
};

template <class Archive>
void GameObjectWrapper::serialize(Archive& ar) {
    if (ptr == nullptr) {
        ptr = new GameObject("TempName", MeshData(), 0);
    }
    ar(*ptr);
};

#endif // GAMEOBJECT_H