#include "Renderer.h"
#include <iostream>
#include <filesystem>
#include <GL/glew.h>
#include <SDL2/SDL_events.h>
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "Importer.h"
#include "GameObject.h"
#include "Variables.h"

extern Camera camera;
extern Importer importer;
extern std::vector<MeshData> meshes;
extern GLuint textureID;

std::string getFileName(const std::string& path) {
    return std::filesystem::path(path).stem().string();
}

void initOpenGL() {
    glewInit();
    if (!GLEW_VERSION_3_0) throw std::exception("OpenGL 3.0 API is not available.");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glClearColor(0.5, 0.5, 0.5, 1.0);
}

bool processEvents(Camera& camera, std::vector<GameObject>& gameObjects, const char*& fbxFile) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type) {
        case SDL_KEYDOWN:
            camera.processKeyDown(event.key.keysym);
            break;

        case SDL_KEYUP:
            camera.processKeyUp(event.key.keysym);
            break;

        case SDL_MOUSEMOTION:
            camera.processMouseMotion(event.motion);
            break;

        case SDL_MOUSEBUTTONDOWN:
            camera.processMouseButtonDown(event.button);
            break;

        case SDL_MOUSEBUTTONUP:
            camera.processMouseButtonUp(event.button);
            break;

        case SDL_MOUSEWHEEL:
            camera.processMouseWheel(event.wheel);
            break;

        case SDL_DROPFILE: {
            fbxFile = event.drop.file;
            meshes.clear();
            textureID = 0;

            meshes = importer.loadFBX(fbxFile, textureID);

            for (size_t i = 0; i < meshes.size(); ++i) {
                const std::string objectName = getFileName(fbxFile) + "_" + std::to_string(i);
                //variables->window->gameObjects.emplace_back(objectName, meshes[i], textureID); 
                GameObject* modelObject = new GameObject(objectName, meshes[i], textureID); 
                variables->window->gameObjects.push_back(modelObject);
                //gameObjects.emplace_back(objectName, meshes[i], textureID);
                //variables->window->getGameObjects().push_back();
            }

            std::string baseName = getFileName(fbxFile);
            std::string outputPath = "Assets/" + baseName + ".dat";
            importer.saveCustomFormat(outputPath, meshes);

            std::cout << "FBX loaded & saved in: " << outputPath << std::endl;
            break;
        }
        case SDL_QUIT:
            return false;
        default:
            break;
        }
    }
    camera.move(SDL_GetKeyboardState(NULL));
    return true;
}

void render(const std::vector<GameObject*>& gameObjects) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 1.0f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    camera.applyCameraTransformations();

    for (const auto& obj : gameObjects) {
        glPushMatrix();

        glm::mat4 transform = obj->getTransformMatrix();
        glMultMatrixf(glm::value_ptr(transform));

        if (obj->textureID) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, obj->textureID);
        }
        else {
            glDisable(GL_TEXTURE_2D);
        }

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, obj->meshData.vertices.data());

        if (!obj->meshData.textCoords.empty()) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, 0, obj->meshData.textCoords.data());
        }

        glDrawElements(GL_TRIANGLES, obj->meshData.indices.size(), GL_UNSIGNED_INT, obj->meshData.indices.data());

        glDisableClientState(GL_VERTEX_ARRAY);

        if (!obj->meshData.textCoords.empty()) {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }

        glPopMatrix();
    }
    glFlush();
}
