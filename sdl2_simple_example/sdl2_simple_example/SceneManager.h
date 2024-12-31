#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "GameObject.h"
#include <filesystem>
#include <unordered_map>
#include <glm/vec3.hpp> 
#include <string>
#include <vector>

struct GameObjectState {
    std::string uuid;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    std::string parentUUID;
    GLuint textureID;  
    bool active;  
    bool dynamic;
};

class SceneManager {
public:
    static SceneManager sceneManager;

    void saveScene(const std::string& outputPath, const std::vector<GameObject*>& gameObjects);
    void loadScene(const std::string& inputPath, std::vector<GameObject*>& gameObjects);
    void listAvailableScenes();

    void saveSceneState(std::vector<GameObject*>& gameObjects);
    void restoreSceneState(std::vector<GameObject*>& gameObjects);

    std::vector<std::string> availableScenes;
private:
    std::vector<GameObjectState> initialState;
};


#endif // SCENEMANAGER_H