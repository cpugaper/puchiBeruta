#include "SimulationManager.h"
#include "ConsoleWindow.h"

SimulationManager SimulationManager::simulationManager;

SimulationManager::SimulationManager()
    : gameObjects(), temporaryObjects(), currentState(SimulationState::Stopped) {
}

SimulationManager::SimulationManager(std::vector<GameObject*>& gameObjects)
    : gameObjects(gameObjects), temporaryObjects(), currentState(SimulationState::Stopped) {
}

SimulationManager::~SimulationManager() {}

void SimulationManager::startSimulation(std::vector<GameObject*>& gameObjects) {
    if (currentState == SimulationState::Stopped || currentState == SimulationState::Paused) {
        if (currentState == SimulationState::Stopped) sceneManager.saveSceneState(gameObjects);
        currentState = SimulationState::Running;
        for (auto& obj : gameObjects) {
            if (obj && obj->movementState != MovementState::Running) {
                obj->startMovement();
            }
        }
        console.addLog("Simulation started.");
    }
}

void SimulationManager::pauseSimulation(std::vector<GameObject*>& gameObjects) {
    if (currentState == SimulationState::Running) {
        currentState = SimulationState::Paused;
        for (auto& obj : gameObjects) {
            if (obj && obj->movementState == MovementState::Running) {
                obj->pauseMovement();
            }
        }
        console.addLog("Simulation paused.");
    }
}

void SimulationManager::stopSimulation(std::vector<GameObject*>& gameObjects) {
    if (currentState == SimulationState::Running || currentState == SimulationState::Paused) {
        currentState = SimulationState::Stopped;
        for (auto& obj : gameObjects) {
            if (obj && obj->movementState != MovementState::Stopped) {
                obj->stopMovement();
            }
        }

        for (auto* tempObj : temporaryObjects) {
            auto it = std::find(gameObjects.begin(), gameObjects.end(), tempObj);
            if (it != gameObjects.end()) {
                gameObjects.erase(it);
                delete tempObj; 
                console.addLog("Temporary GameObject removed: " + tempObj->name);
            }
        }
        temporaryObjects.clear(); 

        console.addLog("Simulation stopped.");
        sceneManager.restoreSceneState(gameObjects);
    }
}

void SimulationManager::update(float deltaTime, std::vector<GameObject*>& gameObjects) {
    console.addLog("Simulation updating BEGINING");
    for (auto& obj : gameObjects) { 
        if (obj->movementState == MovementState::Running) {
            obj->updateMovement(deltaTime);
            console.addLog("GameObject updating");
        }
    }
    console.addLog("Simulation updating END");
}

void SimulationManager::trackObject(GameObject* obj) {
    if (currentState == SimulationState::Running) {
        temporaryObjects.push_back(obj);
        console.addLog("Tracking GameObject created during Running: " + obj->name);
    }
}



SimulationManager::SimulationState SimulationManager::getState() const {
    return currentState;
}

std::string SimulationManager::getStateName(SimulationManager::SimulationState state) {
    switch (state) {
    case SimulationManager::SimulationState::Running:
        return "Running";
    case SimulationManager::SimulationState::Paused:
        return "Paused";
    case SimulationManager::SimulationState::Stopped:
        return "Stopped";
    default:
        return "Unknown";
    }
}