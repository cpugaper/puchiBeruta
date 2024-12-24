#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "GameObject.h"
#include <filesystem>
#include <string>
#include <vector>

class SceneManager {
public:
    static SceneManager sceneManager;

    void saveScene(const std::string& outputPath, const std::vector<GameObject*>& gameObjects);
    void loadScene(const std::string& inputPath, std::vector<GameObject*>& gameObjects);
    void listAvailableScenes();

    std::vector<std::string> availableScenes;
};


#endif // SCENEMANAGER_H
