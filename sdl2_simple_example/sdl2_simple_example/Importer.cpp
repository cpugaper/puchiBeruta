#include "Importer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <IL/ilu.h>
#include <stdexcept>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <chrono>
#include <fstream>
#include "Variables.h"
#include "ConsoleWindow.h"

Importer importer;

// Initialize DevIL and ensures the existence of necessary directories
Importer::Importer() {
    initDevIL();
    checkAndCreateDirectories();
}

Importer::~Importer() {}

void Importer::initDevIL() {
    ilInit();
    iluInit();
}

void Importer::checkAndCreateDirectories() {
    const std::vector<std::string> directories = {
        "Assets",
        "Library/Models",
        "Library/Textures",
        "Library/Materials"
    };

    // Browse the necessary directories and create them if they don't exist
    for (const auto& dir : directories) {
        std::filesystem::path dirPath(dir);
        if (!std::filesystem::exists(dirPath)) {
            std::filesystem::create_directories(dirPath);
        }
    }
    processAssetsToLibrary();
}

void Importer::processAssetsToLibrary() {
    for (const auto& entry : std::filesystem::directory_iterator("Assets")) {
 
        if (entry.is_regular_file()) {

            std::string extension = entry.path().extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

            if (extension == ".fbx") {
                std::string fileName = entry.path().filename().string();
                std::string outputPath = "Library/Models/" + entry.path().stem().string() + ".dat";

                if (!std::filesystem::exists(outputPath)) {
                    GLuint textureID = 0;
                    std::vector<MeshData> meshes = loadFBX(entry.path().string(), textureID);
                    saveCustomFormat(outputPath, meshes);

                    std::filesystem::path texturePathPNG = entry.path().parent_path() /
                        (entry.path().stem().string() + ".png");

                    if (std::filesystem::exists(texturePathPNG)) {
                        processTextureFile(texturePathPNG);
                    }
                }
            }
            else if (extension == ".png") {
                processTextureFile(entry.path());
            }
        }
    }
}

void Importer::processTextureFile(const std::filesystem::path& texturePath) {
    std::string outputPath = "Library/Textures/" + texturePath.stem().string() + ".texdat";

    if (!std::filesystem::exists(outputPath)) {
        saveTextureToCustomFormat(texturePath.string(), outputPath);
        console.addLog("Texture processed: " + texturePath.string());
    }
}

void Importer::saveTextureToCustomFormat(const std::string& inputPath, const std::string& outputPath) {
    TextureData texData = loadTextureData(inputPath);

    std::ofstream file(outputPath, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot create texture file: " + outputPath);

    file.write(reinterpret_cast<const char*>(&texData.width), sizeof(int));
    file.write(reinterpret_cast<const char*>(&texData.height), sizeof(int));
    file.write(reinterpret_cast<const char*>(&texData.channels), sizeof(int));

    size_t dataSize = texData.width * texData.height * texData.channels;
    file.write(reinterpret_cast<const char*>(texData.pixels), dataSize);

    delete[] texData.pixels;
}

TextureData Importer::loadTextureData(const std::string& texturePath) {
    ILuint imageID;
    ilGenImages(1, &imageID);
    ilBindImage(imageID);

    if (!ilLoadImage((const wchar_t*)texturePath.c_str())) {
        throw std::runtime_error("Failed to load texture: " + texturePath);
    }

    TextureData texData;
    texData.width = ilGetInteger(IL_IMAGE_WIDTH);
    texData.height = ilGetInteger(IL_IMAGE_HEIGHT);
    texData.channels = 4;

    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

    size_t dataSize = texData.width * texData.height * texData.channels;
    texData.pixels = new unsigned char[dataSize];
    memcpy(texData.pixels, ilGetData(), dataSize);

    ilDeleteImages(1, &imageID);
    return texData;
}

GLuint Importer::loadTextureFromCustomFormat(const std::string& texturePath) {
    std::ifstream file(texturePath, std::ios::binary);
    if (!file) {
        console.addLog("Error: Cannot open texture file: " + texturePath);
        return 0;
    }

    int width, height, channels;
    file.read(reinterpret_cast<char*>(&width), sizeof(int));
    file.read(reinterpret_cast<char*>(&height), sizeof(int));
    file.read(reinterpret_cast<char*>(&channels), sizeof(int));

    size_t dataSize = width * height * channels;
    unsigned char* pixels = new unsigned char[dataSize];
    file.read(reinterpret_cast<char*>(pixels), dataSize);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    delete[] pixels;
    console.addLog("Custom format texture loaded: " + texturePath);
    return textureID;
}

std::vector<MeshData> Importer::loadModelFromCustomFormat(const std::string& relativeFilePath, GLuint& textureID) {
    console.addLog("Loading model from custom format: " + relativeFilePath);

    std::string currentPath = std::filesystem::current_path().string();
    //std::filesystem::path projectPath = std::filesystem::path(currentPath).parent_path();
    //std::string filePath = (projectPath / relativeFilePath).string();
    std::string filePath =  relativeFilePath;

    if (!std::filesystem::exists(filePath)) {
        console.addLog("Custom format model does not exist: " + filePath);
        return {};
    }

    std::filesystem::path modelPath(filePath);
    std::filesystem::path texturePath = "Library/Textures/" + modelPath.stem().string() + ".texdat";

    if (std::filesystem::exists(texturePath)) {
        textureID = loadTextureFromCustomFormat(texturePath.string());
        console.addLog("Custom format texture found & loaded: " + texturePath.string());
    }
    else {
        console.addLog("Texture not found for " + filePath);
        textureID = 0;
    }

    return loadCustomFormat(filePath);
}

// Loads an FBX model and its corresponding texture
std::vector<MeshData> Importer::loadFBX(const std::string& relativeFilePath, GLuint& textureID) {
    console.addLog("Loading model FBX: " + relativeFilePath);

    std::string currentPath = std::filesystem::current_path().string();
    std::filesystem::path projectPath = std::filesystem::path(currentPath);
    std::string filePath = (projectPath / relativeFilePath).string();

    if (!std::filesystem::exists(filePath)) {
        console.addLog("FBX does not exist: " + filePath);
        return {};
    }

    auto start = std::chrono::high_resolution_clock::now();

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate);

    if (!scene) {
        console.addLog("Error loading FBX: " + std::string(importer.GetErrorString()));
        return {};
    }

    console.addLog("Model loaded with success, number of meshes: " + std::to_string(scene->mNumMeshes));
    std::vector<MeshData> meshes;

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh* aiMesh = scene->mMeshes[i];
        MeshData meshData;

        for (unsigned int v = 0; v < aiMesh->mNumVertices; v++) {
            meshData.vertices.push_back(aiMesh->mVertices[v].x);
            meshData.vertices.push_back(aiMesh->mVertices[v].y);
            meshData.vertices.push_back(aiMesh->mVertices[v].z);

            if (aiMesh->mTextureCoords[0]) {
                meshData.textCoords.push_back(aiMesh->mTextureCoords[0][v].x);
                meshData.textCoords.push_back(1 - aiMesh->mTextureCoords[0][v].y);
            }
        }

        for (unsigned int f = 0; f < aiMesh->mNumFaces; f++) {
            aiFace& face = aiMesh->mFaces[f];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                meshData.indices.push_back(face.mIndices[j]);
            }
        }
        meshes.push_back(meshData);
    }

    std::filesystem::path modelPath(filePath);
    std::filesystem::path texturePath = modelPath.parent_path() / (modelPath.stem().string() + ".png");

    if (std::filesystem::exists(texturePath)) {
        textureID = loadTexture(texturePath.string());
        console.addLog("Texture found & loaded: " + texturePath.string());
    }
    else {
        console.addLog("Texture not found for " + filePath);
        textureID = 0;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    console.addLog("Loading time for FBX: " + std::to_string(elapsed.count()) + " seconds");

    return meshes;
}

GLuint Importer::loadTexture(const std::string& texturePath) {
    ILuint imageID;
    ilGenImages(1, &imageID);
    ilBindImage(imageID);

    if (!ilLoadImage((const wchar_t*)texturePath.c_str())) {
        console.addLog("Error loading texture: " + texturePath);
        return 0;
    }

    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
        ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    ilDeleteImages(1, &imageID);
    console.addLog("Texture loaded: " + texturePath);
    return textureID;
}

void Importer::getTextureDimensions(GLuint textureID, int& width, int& height) {
    glBindTexture(GL_TEXTURE_2D, textureID);

    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
}

void Importer::saveCustomFormat(const std::string& outputPath, const std::vector<MeshData>& meshes) {
    std::ofstream file(outputPath, std::ios::binary);
    if (!file) throw std::runtime_error("File couldn't open for writing");

    size_t meshCount = meshes.size();
    file.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));

    for (const auto& mesh : meshes) {
        size_t verticesSize = mesh.vertices.size();
        file.write(reinterpret_cast<const char*>(&verticesSize), sizeof(verticesSize));
        file.write(reinterpret_cast<const char*>(mesh.vertices.data()), verticesSize * sizeof(GLfloat));

        size_t indicesSize = mesh.indices.size();
        file.write(reinterpret_cast<const char*>(&indicesSize), sizeof(indicesSize));
        file.write(reinterpret_cast<const char*>(mesh.indices.data()), indicesSize * sizeof(GLuint));

        size_t texCoordsSize = mesh.textCoords.size();
        file.write(reinterpret_cast<const char*>(&texCoordsSize), sizeof(texCoordsSize));
        file.write(reinterpret_cast<const char*>(mesh.textCoords.data()), texCoordsSize * sizeof(GLfloat));
    }

    console.addLog("File saved in custom format: " + outputPath);
}

std::vector<MeshData> Importer::loadCustomFormat(const std::string& inputPath) {
    std::ifstream file(inputPath, std::ios::binary);
    if (!file) throw std::runtime_error("File couldn't open for reading");

    std::vector<MeshData> meshes;
    size_t meshCount;
    file.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < meshCount; ++i) {
        MeshData mesh;

        size_t verticesSize;
        file.read(reinterpret_cast<char*>(&verticesSize), sizeof(verticesSize));
        mesh.vertices.resize(verticesSize);
        file.read(reinterpret_cast<char*>(mesh.vertices.data()), verticesSize * sizeof(GLfloat));

        size_t indicesSize;
        file.read(reinterpret_cast<char*>(&indicesSize), sizeof(indicesSize));
        mesh.indices.resize(indicesSize);
        file.read(reinterpret_cast<char*>(mesh.indices.data()), indicesSize * sizeof(GLuint));

        size_t texCoordsSize;
        file.read(reinterpret_cast<char*>(&texCoordsSize), sizeof(texCoordsSize));
        mesh.textCoords.resize(texCoordsSize);
        file.read(reinterpret_cast<char*>(mesh.textCoords.data()), texCoordsSize * sizeof(GLfloat));

        meshes.push_back(mesh);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    console.addLog("Loading time for custom file: " + std::to_string(elapsed.count()) + " seconds");

    return meshes;
}