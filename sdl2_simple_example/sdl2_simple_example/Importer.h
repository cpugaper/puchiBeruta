#ifndef IMPORTER_H
#define IMPORTER_H

#include "GameObject.h"
#include <filesystem>
#include <string>
#include <vector>

class Importer {
public:
    static Importer importer;

    Importer();
    ~Importer();

    std::vector<MeshData> loadModelFromCustomFormat(const std::string& relativeFilePath, GLuint& textureID);
    std::vector<MeshData> loadFBX(const std::string& relativefilePath, GLuint& textureID);
    GLuint loadTexture(const std::string& texturePath);

    void processAssetsToLibrary();
    void loadModel(const std::string& relativeFilePath, GLuint& textureID);

    void saveCustomFormat(const std::string& outputPath, const std::vector<MeshData>& meshes);
    std::vector<MeshData> loadCustomFormat(const std::string& inputPath);

    void getTextureDimensions(GLuint textureID, int& width, int& height);
    /*void saveScene(const std::string& outputPath, const std::vector<MeshData>& meshes);
    std::vector<MeshData> loadScene(const std::string& inputPath);*/

private:
    void initDevIL();
    void checkAndCreateDirectories();
};

#endif // IMPORTER_H