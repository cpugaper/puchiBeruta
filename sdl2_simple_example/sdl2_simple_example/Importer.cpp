// Importer.cpp
#include "Importer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <IL/il.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
using namespace std;

Importer::Importer() {
    initDevIL();
}

Importer::~Importer() {}

void Importer::initDevIL() {
    ilInit();
}

vector<MeshData> Importer::loadFBX(const string& filePath) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate);
    if (!scene) {
        throw runtime_error("Error al cargar el archivo FBX: " + string(importer.GetErrorString()));
    }

    vector<MeshData> meshes;

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

    return meshes;
}

GLuint Importer::loadTexture(const string& texturePath) {
    ILuint imageID;
    ilGenImages(1, &imageID);
    ilBindImage(imageID);

    if (!ilLoadImage((const wchar_t*)texturePath.c_str())) {
        ilDeleteImages(1, &imageID);
        throw runtime_error("Error al cargar la textura.");
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

    ilDeleteImages(1, &imageID);
    return textureID;
}

void Importer::saveCustomFormat(const string& outputPath, const vector<MeshData>& meshes) {
    ofstream file(outputPath, ios::binary);
    if (!file) throw runtime_error("No se pudo abrir el archivo para escribir.");

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
}

vector<MeshData> Importer::loadCustomFormat(const string& inputPath) {
    ifstream file(inputPath, ios::binary);
    if (!file) throw runtime_error("No se pudo abrir el archivo para leer.");

    vector<MeshData> meshes;
    size_t meshCount;
    file.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));

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

    return meshes;
}