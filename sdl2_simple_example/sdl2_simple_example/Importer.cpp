// Importer.cpp
#include "Importer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <IL/il.h>
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <sys/stat.h>
#include <sys/types.h>
#include <chrono>
#include <filesystem>
#include <fstream>

Importer::Importer() {
    initDevIL();
    checkAndCreateDirectories();
}

Importer::~Importer() {}

void Importer::initDevIL() {
    ilInit();
}

void Importer::checkAndCreateDirectories() {
    const std::vector<std::string> directories = {
        "Assets",
        "Library/Meshes",
        "Library/Materials",
        "Library/Models"
    };
    for (const auto& dir : directories) {
        std::filesystem::path dirPath(dir);

        // Create directory if it doesn't exist
        if (!std::filesystem::exists(dirPath)) {
            std::filesystem::create_directories(dirPath);
        }
    }
}

std::vector<MeshData> Importer::loadFBX(const std::string& relativeFilePath, GLuint& textureID) {

    std::string currentPath = std::filesystem::current_path().string();
    std::filesystem::path projectPath = std::filesystem::path(currentPath).parent_path();
    std::string filePath = (projectPath / relativeFilePath).string();

    if (!std::filesystem::exists(filePath)) {
        throw std::runtime_error("El archivo FBX no existe: " + filePath);
    }

    auto start = std::chrono::high_resolution_clock::now();

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate);
    if (!scene) {
        throw std::runtime_error("Error al cargar el archivo FBX: " + std::string(importer.GetErrorString()));
    }

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
        textureID = loadTexture(texturePath.string()); // Load Texture
        std::cout << "Textura encontrada y cargada: " << texturePath << std::endl;
    }
    else {
        std::cout << "No se encontr� una textura para " << filePath << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Tiempo de carga del archivo FBX: " << elapsed.count() << " segundos" << std::endl;

    return meshes;
}

GLuint Importer::loadTexture(const std::string& texturePath) {
    ILuint imageID;
    ilGenImages(1, &imageID);
    ilBindImage(imageID);

    if (!ilLoadImage((const wchar_t*)texturePath.c_str())) {
        ilDeleteImages(1, &imageID);
        throw std::runtime_error("Error al cargar la textura.");
    }

    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);

    ilDeleteImages(1, &imageID);
    return textureID;
}

void Importer::saveCustomFormat(const std::string& outputPath, const std::vector<MeshData>& meshes) {
    std::ofstream file(outputPath, std::ios::binary);
    if (!file) throw std::runtime_error("No se pudo abrir el archivo para escribir.");

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

    std::cout << "Archivo guardado en formato personalizado: " << outputPath << std::endl;
}

std::vector<MeshData> Importer::loadCustomFormat(const std::string& inputPath) {
    std::ifstream file(inputPath, std::ios::binary);
    if (!file) throw std::runtime_error("No se pudo abrir el archivo para leer.");

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

    std::cout << "Tiempo de carga de archivo personalizado: " << elapsed.count() << " segundos" << std::endl;

    return meshes;
}