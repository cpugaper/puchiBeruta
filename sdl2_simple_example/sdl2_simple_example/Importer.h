#ifndef IMPORTER_H
#define IMPORTER_H

#include "GameObject.h"
#include <filesystem>
#include <string>
#include <vector>

class Importer {
public:
    Importer();
    ~Importer();

    std::vector<MeshData> loadFBX(const std::string& relativefilePath, GLuint& textureID);
    GLuint loadTexture(const std::string& texturePath);

    void saveCustomFormat(const std::string& outputPath, const std::vector<MeshData>& meshes);
    std::vector<MeshData> loadCustomFormat(const std::string& inputPath);

    /*void saveScene(const std::string& outputPath, const std::vector<MeshData>& meshes);
    std::vector<MeshData> loadScene(const std::string& inputPath);*/

private:
    void initDevIL();
    void checkAndCreateDirectories();
};

#endif // IMPORTER_H