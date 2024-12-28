#ifndef IMPORTER_H
#define IMPORTER_H
#include "GameObject.h"
#include <filesystem>
#include <string>
#include <vector>

struct TextureData {
    unsigned char* pixels;
    int width;
    int height;
    int channels;
};

class Importer {
public:
    static Importer importer;
    Importer();
    ~Importer();

    // Modelo y carga
    std::vector<MeshData> loadModelFromCustomFormat(const std::string& relativeFilePath, GLuint& textureID);
    std::vector<MeshData> loadFBX(const std::string& relativefilePath, GLuint& textureID);
    void processAssetsToLibrary();
    void saveCustomFormat(const std::string& outputPath, const std::vector<MeshData>& meshes);
    std::vector<MeshData> loadCustomFormat(const std::string& inputPath);

    // Textura y utilidades
    GLuint loadTexture(const std::string& texturePath);
    GLuint loadTextureFromCustomFormat(const std::string& texturePath);
    void getTextureDimensions(GLuint textureID, int& width, int& height);

    // Nuevos métodos para el manejo de texturas
    void saveTextureToCustomFormat(const std::string& inputPath, const std::string& outputPath);
    TextureData loadTextureData(const std::string& texturePath);
    void processTextureFile(const std::filesystem::path& texturePath);

private:
    void initDevIL();
    void checkAndCreateDirectories();
};
#endif // IMPORTER_H