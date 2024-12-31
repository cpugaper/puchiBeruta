#ifndef SIMULATIONMANAGER_H
#define SIMULATIONMANAGER_H

#include "SceneManager.h"

class SimulationManager {
public:
    static SimulationManager simulationManager;

    enum class SimulationState {
        Stopped,
        Running,
        Paused
    };

    SimulationManager();
    SimulationManager(std::vector<GameObject*>& gameObjects);
    ~SimulationManager();

    void startSimulation(std::vector<GameObject*>& gameObjects);
    void pauseSimulation(std::vector<GameObject*>& gameObjects);
    void stopSimulation(std::vector<GameObject*>& gameObjects);
    void update(float deltaTime, std::vector<GameObject*>& gameObjects);

    SimulationState getState() const;
    std::string getStateName(SimulationManager::SimulationState state);

    void trackObject(GameObject* obj);

private:
    std::vector<GameObject*> gameObjects;
    std::vector<GameObject*> temporaryObjects;
    SimulationState currentState;
    SceneManager sceneManager; 
};

#endif SIMULATIONMANAGER_H