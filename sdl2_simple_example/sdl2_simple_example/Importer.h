#ifndef IMPORTER_H
#define IMPORTER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <filesystem>
#include <string>
#include <vector>

struct MeshData {
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    std::vector<GLfloat> textCoords;
    std::vector<GLfloat> normals;
};

class Importer {
public:
    Importer();
    ~Importer();

    std::vector<MeshData> loadFBX(const std::string& relativefilePath, GLuint& textureID);
    GLuint loadTexture(const std::string& texturePath);

    void saveCustomFormat(const std::string& outputPath, const std::vector<MeshData>& meshes);
    std::vector<MeshData> loadCustomFormat(const std::string& inputPath);

private:
    void initDevIL();
    void checkAndCreateDirectories();
};

#endif // IMPORTER_H