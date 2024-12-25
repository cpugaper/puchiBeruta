#include "SimulationManager.h"
#include "ConsoleWindow.h"

SimulationManager SimulationManager::simulationManager;

SimulationManager::SimulationManager()
    : gameObjects(), currentState(SimulationState::Stopped) {
}

SimulationManager::SimulationManager(std::vector<GameObject*>& gameObjects)
    : gameObjects(gameObjects), currentState(SimulationState::Stopped) {
}

SimulationManager::~SimulationManager() {}

void SimulationManager::startSimulation() {
    if (currentState == SimulationState::Stopped || currentState == SimulationState::Paused) {
        currentState = SimulationState::Running;
        for (auto& obj : gameObjects) {
            if (obj) {
                obj->startMovement();
            }
        }
        sceneManager.saveSceneState(gameObjects); 
        console.addLog("Simulation started.");
    }
}

void SimulationManager::pauseSimulation() {
    if (currentState == SimulationState::Running) {
        currentState = SimulationState::Paused;
        for (auto& obj : gameObjects) {
            if (obj) {
                obj->pauseMovement();
            }
        }
        console.addLog("Simulation paused.");
    }
}

void SimulationManager::stopSimulation() {
    if (currentState == SimulationState::Running || currentState == SimulationState::Paused) {
        currentState = SimulationState::Stopped;
        for (auto& obj : gameObjects) {
            if (obj) {
                obj->stopMovement();
            }
        }
        sceneManager.restoreSceneState(gameObjects); 
        console.addLog("Simulation stopped.");
        resetSimulation();
    }
}

void SimulationManager::resetSimulation() {
    if (currentState == SimulationState::Stopped) {
        sceneManager.restoreSceneState(gameObjects); 
        console.addLog("Simulation reset.");
    }
}

void SimulationManager::update(float deltaTime) {
    for (auto& obj : gameObjects) {
        if (obj->movementState == MovementState::Running) {
            obj->updateMovement(deltaTime);  
        }
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
