#pragma once
#include <vector>
#include <string>
#include "Importer.h"
using namespace std;

class GameObject {
public:
    string name;
    vector<GameObject> children;
    MeshData meshData; // Incluye datos de la malla

    GameObject(const string& name, const MeshData& mesh) : name(name), meshData(mesh) {}

    void addChild(const GameObject& child) {
        children.push_back(child);
    }
};
