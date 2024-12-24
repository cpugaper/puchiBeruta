#include "SceneManager.h"
#include "ConsoleWindow.h"
#include <fstream>

SceneManager sceneManager;

void SceneManager::saveScene(const std::string& outputPath, const std::vector<GameObject*>& gameObjects) {
    std::ofstream outFile(outputPath);
    if (!outFile) {
        console.addLog("Error opening file for writing: " + outputPath);
        return;
    }

    try {
        cereal::JSONOutputArchive archive(outFile);

        std::vector<GameObjectWrapper> wrappedObjects;
        wrappedObjects.reserve(gameObjects.size());
        for (auto* obj : gameObjects) {
            wrappedObjects.emplace_back(obj);
        }

        archive(cereal::make_nvp("gameObjects", wrappedObjects));
        console.addLog("Scene saved successfully to: " + outputPath);
    }
    catch (const std::exception& e) {
        console.addLog("Error saving scene: " + std::string(e.what()));
    }
}

void SceneManager::loadScene(const std::string& inputPath, std::vector<GameObject*>& gameObjects) {
    std::ifstream inFile(inputPath);

    if (!inFile) {
        console.addLog("Error opening file for reading: " + inputPath);
        return;
    }

    try {
        cereal::JSONInputArchive archive(inFile);

        for (auto obj : gameObjects) {
            delete obj;
        }
        gameObjects.clear();

        std::vector<GameObjectWrapper> wrappedObjects;
        archive(cereal::make_nvp("gameObjects", wrappedObjects));

        std::unordered_map<std::string, GameObject*> uuidToGameObject;
        gameObjects.reserve(wrappedObjects.size());

        for (auto& wrapper : wrappedObjects) {
            GameObject* obj = wrapper.ptr;
            gameObjects.push_back(obj);
            uuidToGameObject[obj->uuid] = obj;

            obj->initialPosition = obj->position;
            obj->initialRotation = obj->rotation;
            obj->initialScale = obj->scale;

            obj->globalTransform = obj->getTransformMatrix();

            obj->loadTextureFromPath();

            console.addLog("Loaded GameObject: " + obj->name + " UUID: " + obj->uuid);
        }

        for (auto obj : gameObjects) {
            for (const auto& childUUID : obj->pendingChildUUIDs) {
                auto it = uuidToGameObject.find(childUUID);
                if (it != uuidToGameObject.end()) {
                    obj->addChild(it->second);
                }
                else {
                    console.addLog("Warning: Child UUID not found: " + childUUID);
                }
            }
            obj->pendingChildUUIDs.clear();
        }
        for (auto obj : gameObjects) {
            if (obj->parent == nullptr) {
                obj->updateChildTransforms();
            }
        }
        console.addLog("Scene loaded successfully from: " + inputPath);
    }
    catch (const cereal::Exception e) {
        console.addLog("Error deserializing scene: " + std::string(e.what()));
    }
    catch (const std::exception& e) {
        console.addLog("Error loading scene: " + std::string(e.what()));
    }
}

void SceneManager::listAvailableScenes() {
    availableScenes.clear();
    std::string directory = "Assets/Scenes";

    if (std::filesystem::exists(directory)) {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                availableScenes.push_back(entry.path().filename().string());
            }
        }
    }
}

