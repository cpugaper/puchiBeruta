#pragma once
#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
using namespace std;

struct MeshData {
    vector<GLfloat> vertices;
    vector<GLuint> indices;
    vector<GLfloat> textCoords;
};

class Importer {
public:
    Importer();
    ~Importer();

    vector<MeshData> loadFBX(const string& filePath);
    GLuint loadTexture(const string& texturePath);

    void saveCustomFormat(const string& outputPath, const vector<MeshData>& meshes);
    vector<MeshData> loadCustomFormat(const string& inputPath);

private:
    void initDevIL();
};
