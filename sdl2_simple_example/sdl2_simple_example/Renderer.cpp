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
#include "ConsoleWindow.h"
#include "SceneWindow.h"

extern Camera camera;
extern Importer importer;
extern SceneWindow sceneWindow;
extern std::vector<MeshData> meshes;
extern GLuint textureID;
GLuint framebuffer = 0;
GLuint textureColorbuffer = 0;
GLuint rbo = 0;
int framebufferWidth = 1280;
int framebufferHeight = 720;

Renderer renderer;

// Gets the filename of a given path
std::string Renderer::getFileName(const std::string& path) {
    return std::filesystem::path(path).stem().string();
}

//Initialises LWE and configures OpenGL for deep rendering
void Renderer::initOpenGL() {
    glewInit();
    if (!GLEW_VERSION_3_0) throw std::exception("OpenGL 3.0 API is not available.");
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5, 0.5, 0.5, 1.0);
}

void Renderer::createFrameBuffer(int width, int height) {
    framebufferWidth = width;
    framebufferHeight = height;

    // Removy any previous framebuffers before creating a new one
    if (framebuffer != 0) {
        cleanupFrameBuffer();
    }

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        console.addLog("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Processes SDL events and user actions
bool Renderer::processEvents(Camera& camera, std::vector<GameObject>& gameObjects, const char*& droppedFilePath) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

        if (!sceneWindow.isActive) {
            continue; 
        }

        switch (event.type) {
        case SDL_KEYDOWN: camera.processKeyDown(event.key.keysym); break;
        case SDL_KEYUP: camera.processKeyUp(event.key.keysym); break;
        case SDL_MOUSEMOTION: camera.processMouseMotion(event.motion); break;
        case SDL_MOUSEBUTTONDOWN: camera.processMouseButtonDown(event.button); break;
        case SDL_MOUSEBUTTONUP: camera.processMouseButtonUp(event.button); break;
        case SDL_MOUSEWHEEL: camera.processMouseWheel(event.wheel); break;

        case SDL_DROPFILE: {
            const char* droppedFilePath = event.drop.file;

            if (droppedFilePath) {
                HandleDroppedFile(droppedFilePath);
                SDL_free(event.drop.file);
            }
            break;
        }

        // Handles the window resize event and adjusts the viewport
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                variables->windowWidth = event.window.data1;
                variables->windowHeight = event.window.data2;
                glViewport(0, 0, Variables::WINDOW_SIZE.x, Variables::WINDOW_SIZE.y);
            }
            break;
        case SDL_QUIT:
            return false;
        default:
            break;
        }
    }
    camera.move(SDL_GetKeyboardState(nullptr));
    return true;
}

void Renderer::HandleDroppedFile(const char* droppedFile) {
    std::filesystem::path filePath(droppedFile);
    std::string extension = filePath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    if (extension == ".dat") {
        try {
            // Cargar meshes desde el archivo .dat
            meshes = importer.loadCustomFormat(filePath.string());

            // Buscar textura asociada en Library/Textures
            std::filesystem::path texturePathPNG = std::filesystem::path("Library/Textures") /
                (filePath.stem().string() + ".png");

            GLuint textureID = 0;
            std::string texturePathString;
            if (std::filesystem::exists(texturePathPNG)) {
                textureID = importer.loadTexture(texturePathPNG.string());
                texturePathString = texturePathPNG.string();
            }

            // Crear objetos de juego para cada mesh
            for (size_t i = 0; i < meshes.size(); ++i) {
                const std::string objectName = getFileName(filePath.string()) + "_" + std::to_string(i);
                GameObject* newObject = new GameObject(objectName, meshes[i], textureID, texturePathString);
                variables->window->gameObjects.push_back(newObject);
            }

            console.addLog("DAT model loaded: " + filePath.string());
        }
        catch (const std::exception& e) {
            console.addLog("Error loading DAT file: " + std::string(e.what()));
        }
    }

    // Si es un FBX, usa el método de importación de FBX
    else if (extension == ".fbx") {
        meshes = importer.loadFBX(droppedFile, textureID);

        for (size_t i = 0; i < meshes.size(); ++i) {
            const std::string objectName = getFileName(droppedFile) + "_" + std::to_string(i);
            GameObject* newObject = new GameObject(objectName, meshes[i], textureID);
            variables->window->gameObjects.push_back(newObject);
        }

        std::string outputPath = "Library/Models/" + getFileName(droppedFile) + ".dat";
        importer.saveCustomFormat(outputPath, meshes);

        console.addLog("FBX model loaded and saved in: " + outputPath);
        
    }

    else if (extension == ".png" || extension == ".dds" || extension == ".texdat") {
        GLuint newTextureID;

        // Determinar qué método de carga usar según la extensión
        if (extension == ".texdat") {
            newTextureID = importer.loadTextureFromCustomFormat(droppedFile);
        }
        else {
            newTextureID = importer.loadTexture(droppedFile);
        }

        std::string textureFilePath = filePath.string();

        if (variables->window->selectedObject) {
            // Si había una textura previa, la liberamos
            if (variables->window->selectedObject->textureID != 0) {
                glDeleteTextures(1, &variables->window->selectedObject->textureID);
            }

            variables->window->selectedObject->textureID = newTextureID;
            variables->window->selectedObject->texturePath = textureFilePath;
            console.addLog("Texture applied to selected object: " + textureFilePath);
        }
        else {
            console.addLog("No object selected to apply the texture.");
            // Limpiamos la textura si no hay objeto seleccionado
            if (newTextureID != 0) {
                glDeleteTextures(1, &newTextureID);
            }
        }
    }
    else {
        console.addLog("Unsupported file type: " + extension);
    }
}

void Renderer::HandleDragDropTarget() {
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LibraryFile")) {
            const char* droppedFile = static_cast<const char*>(payload->Data);
            if (droppedFile) {
                HandleDroppedFile(droppedFile);
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void Renderer::drawGrid(float spacing) {
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.7f, 0.7f, 0.7f);

    float gridRange = 1000.0f;
    glBegin(GL_LINES);

    for (float i = -gridRange; i <= gridRange; i += spacing) {
        glVertex3f(i, 0, -gridRange);
        glVertex3f(i, 0, gridRange);
        glVertex3f(-gridRange, 0, i);
        glVertex3f(gridRange, 0, i);
    }

    glEnd();
}

void Renderer::render(const std::vector<GameObject*>& gameObjects) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, static_cast<float>(framebufferWidth) / framebufferHeight, 0.1f, 100.0f);
    glScalef(1.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    camera.applyCameraTransformations();

    drawGrid(0.5f);

    // Render every object in the scene
    for (const auto& obj : gameObjects) {
        if (!obj->getActive()) {
            continue; 
        }
        glPushMatrix();

        glm::mat4 transform = obj->getTransformMatrix();
        glMultMatrixf(glm::value_ptr(transform));

        glColor3f(1.0f, 1.0f, 1.0f);

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
    /*if (sceneWindow.rayoexists) {
        sceneWindow.DrawRay(*sceneWindow.rayo, 1000);
    }*/
    

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, Variables::WINDOW_SIZE.x, Variables::WINDOW_SIZE.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glFlush();
}

void Renderer::cleanupFrameBuffer() {
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &textureColorbuffer);
    glDeleteRenderbuffers(1, &rbo);
}
