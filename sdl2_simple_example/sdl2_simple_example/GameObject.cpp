#include "GameObject.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <random>
#include <fstream>
#include "Importer.h"
#include "ConsoleWindow.h"
#include "SimulationManager.h"

extern Importer importer;
std::vector<GameObject> gameObjects;

GameObject::GameObject(const std::string& name, const MeshData& mesh, GLuint texID, const std::string& texPath)
    : name(name)
    , meshData(mesh)
    , textureID(texID)
    , texturePath(texPath)
    , position(0.0f)
    , rotation(0.0f)
    , scale(1.0f)
    , uuid(GenerateUUID())
    , elapsedPausedTime(0.0f)
    , parent(nullptr)
    , movementState(MovementState::Stopped)
    , active(true) {
    setTexture(texPath, texID);
    initialPosition = position;
    initialRotation = rotation;
    initialScale = scale;
    console.addLog("GameObject created with UUID: " + uuid);
}

void GameObject::updateMovement(float deltaTime) {
    if (dynamic && movementState == MovementState::Running) {
        position.x += movementDirection * speed * deltaTime;
        if (position.x >= movementRange.second || position.x <= movementRange.first) {
            movementDirection *= -1; 
        }
        console.addLog("Dynamic: " + std::to_string(dynamic));

    }
}

void GameObject::startMovement() {
    if (movementState != MovementState::Running) {
        movementState = MovementState::Running;
        startTime = std::chrono::high_resolution_clock::now() - std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::duration<float>(elapsedPausedTime));
        console.addLog(name + " started moving.");
    }
}

void GameObject::pauseMovement() {
    if (movementState == MovementState::Running) {
        movementState = MovementState::Paused;
        auto currentTime = std::chrono::high_resolution_clock::now();
        elapsedPausedTime = std::chrono::duration<float>(currentTime - startTime).count();
        console.addLog(name + " paused movement.");
    }
}

void GameObject::stopMovement() {
    if (movementState != MovementState::Stopped) {
        movementState = MovementState::Stopped;
        elapsedPausedTime = 0.0f; 
        movementDirection = 1.0f;
        console.addLog(name + " stopped and reset position.");
    }
}


// Generate a random UUID to identify each object uniquely
std::string GameObject::GenerateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;

    for (int i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4"; // Version 4 UUID
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 12; i++) {
        ss << dis(gen);
    }
    return ss.str();
}

void GameObject::setTexture(const std::string& path, GLuint texID) {
    texturePath = path;
    textureID = texID;
}

void GameObject::loadTextureFromPath() {
    if (!texturePath.empty()) {
        textureID = importer.loadTexture(texturePath);
        if (textureID == 0) {
            console.addLog("Failed to load texture from: " + texturePath);
        }
    }
    else {
        console.addLog("Texture path is empty for GameObject: " + name);
    }
}

// Adds a child object to the list of children of this object
void GameObject::addChild(GameObject* child) {
    child->parent = this;

    child->initialPosition = child->position - this->position;  
    child->initialRotation = child->rotation - this->rotation; 
    child->initialScale = child->scale / this->scale;

    children.push_back(child);

    updateChildTransforms();
}

void GameObject::removeChild(GameObject* child) {
    auto it = std::find(children.begin(), children.end(), child);

    if (it != children.end()) {
        children.erase(it);
        child->parent = nullptr;
    }
}

void GameObject::createPrimitive(const std::string& primitiveType, std::vector<GameObject*>& gameObjects) {
    MeshData meshData;
    GLuint textureID = 0;
    std::string filePath = "Assets/Primitives/";

    if (primitiveType == "Sphere") {
        filePath += "sphere.fbx";
    }
    else if (primitiveType == "Cube") {
        filePath += "cube.fbx";
    }
    else if (primitiveType == "Cylinder") {
        filePath += "cylinder.fbx";
    }
    else if (primitiveType == "Cone") {
        filePath += "cone.fbx";
    }
    else if (primitiveType == "Torus") {
        filePath += "torus.fbx";
    }
    else if (primitiveType == "Plane") {
        filePath += "plane.fbx";
    }

    try {
        std::vector<MeshData> meshes = importer.loadFBX(filePath, textureID);
        if (!meshes.empty()) {
            meshData = meshes[0];
            GameObject* modelObject = new GameObject(primitiveType, meshData, textureID);
            gameObjects.push_back(modelObject);
            SimulationManager::simulationManager.trackObject(modelObject);
            console.addLog("Model " + primitiveType + " loaded from " + filePath);
        }
    }
    catch (const std::exception& e) {
        console.addLog("Error when loading model " + primitiveType + ": " + e.what());
    }
}

void GameObject::createEmptyObject(const std::string& name, std::vector<GameObject*>& gameObjects) {
    MeshData emptyMeshData;
    GLuint emptyTextureID = 0;
    GameObject* emptyObject = new GameObject(name, emptyMeshData, emptyTextureID);

    gameObjects.push_back(emptyObject);
    SimulationManager::simulationManager.trackObject(emptyObject);
    console.addLog("Empty object created");
}

void GameObject::createCameraObject(const std::string& name, std::vector<GameObject*>& gameObjects) {
    MeshData emptyMeshData;
    GLuint emptyTextureID = 0;
    GameObject* emptyObject = new GameObject(name, emptyMeshData, emptyTextureID);

    emptyObject->isCamera = true;

    gameObjects.push_back(emptyObject);
    SimulationManager::simulationManager.trackObject(emptyObject);
    console.addLog("Camera object created");

    //emptyObject->RegenerateCorners();

    // Llamamos al método para dibujar la bounding box en el origen (0, 0, 0)
    emptyObject->DrawBoundingBox();
}
void GameObject::DrawBoundingBox() {
    glPushMatrix();  // Guardamos el estado actual de la matriz de transformación

    // Posicionamos el cubo en el origen (0, 0, 0)
    glTranslatef(0.0f, 0.0f, 0.0f);  // Esto no mueve el cubo porque ya está en (0, 0, 0)

    glColor3f(1.0f, 0.0f, 0.0f);  // Color rojo para el cubo

    glBegin(GL_LINES);  // Empezamos a dibujar líneas

    // Definimos los vértices del cubo (tamaño 1)
    float size = 1.0f;  // Tamaño del cubo

    // Vértices del cubo
    glm::vec3 v0(-size / 2, -size / 2, -size / 2);
    glm::vec3 v1(size / 2, -size / 2, -size / 2);
    glm::vec3 v2(size / 2, size / 2, -size / 2);
    glm::vec3 v3(-size / 2, size / 2, -size / 2);
    glm::vec3 v4(-size / 2, -size / 2, size / 2);
    glm::vec3 v5(size / 2, -size / 2, size / 2);
    glm::vec3 v6(size / 2, size / 2, size / 2);
    glm::vec3 v7(-size / 2, size / 2, size / 2);

    // Dibujamos las 12 líneas del cubo (conectando los vértices)
    glVertex3f(v0.x, v0.y, v0.z); glVertex3f(v1.x, v1.y, v1.z);  // Línea 1
    glVertex3f(v1.x, v1.y, v1.z); glVertex3f(v2.x, v2.y, v2.z);  // Línea 2
    glVertex3f(v2.x, v2.y, v2.z); glVertex3f(v3.x, v3.y, v3.z);  // Línea 3
    glVertex3f(v3.x, v3.y, v3.z); glVertex3f(v0.x, v0.y, v0.z);  // Línea 4

    glVertex3f(v4.x, v4.y, v4.z); glVertex3f(v5.x, v5.y, v5.z);  // Línea 5
    glVertex3f(v5.x, v5.y, v5.z); glVertex3f(v6.x, v6.y, v6.z);  // Línea 6
    glVertex3f(v6.x, v6.y, v6.z); glVertex3f(v7.x, v7.y, v7.z);  // Línea 7
    glVertex3f(v7.x, v7.y, v7.z); glVertex3f(v4.x, v4.y, v4.z);  // Línea 8

    glVertex3f(v0.x, v0.y, v0.z); glVertex3f(v4.x, v4.y, v4.z);  // Línea 9
    glVertex3f(v1.x, v1.y, v1.z); glVertex3f(v5.x, v5.y, v5.z);  // Línea 10
    glVertex3f(v2.x, v2.y, v2.z); glVertex3f(v6.x, v6.y, v6.z);  // Línea 11
    glVertex3f(v3.x, v3.y, v3.z); glVertex3f(v7.x, v7.y, v7.z);  // Línea 12

    glEnd();  // Terminamos de dibujar las líneas

    glPopMatrix();  // Restauramos el estado de la matriz de transformación
}
glm::mat4 GameObject::getTransformMatrix() const {
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0, 0, 1));
    transform = glm::scale(transform, scale);
    return transform;
}

// Transforms for parenting logic

glm::mat4 GameObject::getFinalTransformMatrix() const {
    glm::mat4 localTransform = getTransformMatrix();

    if (parent != nullptr) {
        return parent->getFinalTransformMatrix() * localTransform;
    }
    return localTransform;
}

void GameObject::updateChildTransforms() {
    for (GameObject* child : children) {
        child->position = this->position + child->initialPosition;
        child->rotation = this->rotation + child->initialRotation;
        child->scale = this->scale * child->initialScale;

        child->updateChildTransforms();
    }
}

void GameObject::setPosition(const glm::vec3& newPosition) {
    position = newPosition;
    RegenerateCorners();
}

void GameObject::setRotation(const glm::vec3& newRotation) {
    rotation = newRotation;
    //RegenerateCorners();
}

void GameObject::setScale(const glm::vec3& newScale) {
    scale = newScale;
    RegenerateCorners();
}

void GameObject::resetTransform() {
    position = initialPosition;
    rotation = initialRotation;
    scale = initialScale;

    movementDirection = 1.0f;

    if (parent != nullptr) {
        parent->updateChildTransforms();
    }
}

void GameObject::BoundingBoxGeneration() {
    console.addLog("Doing bounding box !!!");
    MeshData* meshData = getMeshData();
    if (meshData) {
        glm::vec3 minLocal(FLT_MAX, FLT_MAX, FLT_MAX);
        glm::vec3 maxLocal(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        for (size_t i = 0; i < meshData->vertices.size(); i += 3) {
            glm::vec3 vertex(meshData->vertices[i], meshData->vertices[i + 1], meshData->vertices[i + 2]);
            minLocal = glm::min(minLocal, vertex);
            maxLocal = glm::max(maxLocal, vertex);
        }
        boundingBoxMinLocal = minLocal;
        boundingBoxMaxLocal = maxLocal; 
    }

    RegenerateCorners();
}
void GameObject::DrawVertex() {
    console.addLog("Entra en la funcion de dibujar los vertices");

    MeshData* meshData = getMeshData();
    if (meshData) {
        glLineWidth(2.0f);  // Establecer el grosor de la línea
        glBegin(GL_LINES);  // Empezamos a dibujar líneas
        glBindTexture(GL_TEXTURE_2D, 0);  // No usar textura

        for (size_t i = 0; i < meshData->vertices.size(); i += 3) {
            glm::vec3 vertex1(meshData->vertices[i], meshData->vertices[i + 1], meshData->vertices[i + 2]);

            // Dibujamos una línea entre cada par de vértices consecutivos
            if (i + 3 < meshData->vertices.size()) {  // Asegúrate de que haya un siguiente vértice
                glm::vec3 vertex2(meshData->vertices[i + 3], meshData->vertices[i + 4], meshData->vertices[i + 5]);

                glColor3f(0.0f, 1.0f, 0.0f);  // Color verde
                glVertex3f(vertex1.x, vertex1.y, vertex1.z);  // Primer vértice
                glVertex3f(vertex2.x, vertex2.y, vertex2.z);  // Segundo vértice
            }
        }

        glEnd();  // Terminamos de dibujar las líneas
    }
}

void GameObject::RegenerateCorners()
{
    //rotation = glm::radians(rotation); 

    glm::mat4 transMatrix = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotMatrix = glm::mat4(1.0f);
    rotMatrix = glm::rotate(rotMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotación en X
    rotMatrix = glm::rotate(rotMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotación en Y
    rotMatrix = glm::rotate(rotMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotación en Z
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

    //return transMatrix * rotMatrix * scaleMatrix; 
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(transMatrix * rotMatrix * scaleMatrix));

    glm::vec3 temp[8] = {
         glm::vec3(boundingBoxMinLocal.x, boundingBoxMinLocal.y, boundingBoxMinLocal.z),  // V0
         glm::vec3(boundingBoxMaxLocal.x, boundingBoxMinLocal.y, boundingBoxMinLocal.z),  // V1
         glm::vec3(boundingBoxMinLocal.x, boundingBoxMaxLocal.y, boundingBoxMinLocal.z),  // V2
         glm::vec3(boundingBoxMaxLocal.x, boundingBoxMaxLocal.y, boundingBoxMinLocal.z),  // V3
         glm::vec3(boundingBoxMinLocal.x, boundingBoxMinLocal.y, boundingBoxMaxLocal.z),  // V4
         glm::vec3(boundingBoxMaxLocal.x, boundingBoxMinLocal.y, boundingBoxMaxLocal.z),  // V5
         glm::vec3(boundingBoxMinLocal.x, boundingBoxMaxLocal.y, boundingBoxMaxLocal.z),  // V6
         glm::vec3(boundingBoxMaxLocal.x, boundingBoxMaxLocal.y, boundingBoxMaxLocal.z)   // V7
    };

    std::copy(std::begin(temp), std::end(temp), corners);
    
    // Establecer el color de la bounding box (amarillo)
    glColor3f(1.0f, 1.0f, 0.0f);

    // Dibujar las 12 aristas de la bounding box usando líneas
    glBegin(GL_LINES);

    // Parte inferior (aristas del plano minWorld.z)
    glVertex3fv(glm::value_ptr(corners[0])); glVertex3fv(glm::value_ptr(corners[1])); // V0 ↔ V1
    glVertex3fv(glm::value_ptr(corners[1])); glVertex3fv(glm::value_ptr(corners[3])); // V1 ↔ V3
    glVertex3fv(glm::value_ptr(corners[3])); glVertex3fv(glm::value_ptr(corners[2])); // V3 ↔ V2
    glVertex3fv(glm::value_ptr(corners[2])); glVertex3fv(glm::value_ptr(corners[0])); // V2 ↔ V0

    // Parte superior (aristas del plano maxWorld.z)
    glVertex3fv(glm::value_ptr(corners[4])); glVertex3fv(glm::value_ptr(corners[5])); // V4 ↔ V5
    glVertex3fv(glm::value_ptr(corners[5])); glVertex3fv(glm::value_ptr(corners[7])); // V5 ↔ V7
    glVertex3fv(glm::value_ptr(corners[7])); glVertex3fv(glm::value_ptr(corners[6])); // V7 ↔ V6
    glVertex3fv(glm::value_ptr(corners[6])); glVertex3fv(glm::value_ptr(corners[4])); // V6 ↔ V4

    // Conexiones verticales (vértices inferiores a los superiores)
    glVertex3fv(glm::value_ptr(corners[0])); glVertex3fv(glm::value_ptr(corners[4])); // V0 ↔ V4
    glVertex3fv(glm::value_ptr(corners[1])); glVertex3fv(glm::value_ptr(corners[5])); // V1 ↔ V5
    glVertex3fv(glm::value_ptr(corners[2])); glVertex3fv(glm::value_ptr(corners[6])); // V2 ↔ V6
    glVertex3fv(glm::value_ptr(corners[3])); glVertex3fv(glm::value_ptr(corners[7])); // V3 ↔ V7

    glEnd(); // Terminar de dibujar las líneas

    // Restaurar la matriz después de dibujar
    glPopMatrix();
}