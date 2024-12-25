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

    void startSimulation();
    void pauseSimulation();
    void stopSimulation();
    void resetSimulation();

    void update(float deltaTime);

    SimulationState getState() const;
    std::string getStateName(SimulationManager::SimulationState state);

private:
    std::vector<GameObject*> gameObjects;
    SimulationState currentState;
    SceneManager sceneManager; 
};

#endif SIMULATIONMANAGER_H
