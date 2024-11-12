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

extern Camera camera;
extern Importer importer;
extern std::vector<MeshData> meshes;
extern GLuint textureID;
GLuint framebuffer = 0;
GLuint textureColorbuffer = 0;
GLuint rbo = 0;
int framebufferWidth = 1280; 
int framebufferHeight = 720;

// Gets the filename of a given path
std::string getFileName(const std::string& path) {
    return std::filesystem::path(path).stem().string();
}

//Initialises LWE and configures OpenGL for deep rendering
void initOpenGL() {
    glewInit();
    if (!GLEW_VERSION_3_0) throw std::exception("OpenGL 3.0 API is not available.");
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5, 0.5, 0.5, 1.0);
}

void createFrameBuffer(int width, int height) {
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
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

// Processes SDL events and user actions
bool processEvents(Camera& camera, std::vector<GameObject>& gameObjects, const char*& droppedFilePath) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type) {
        case SDL_KEYDOWN: camera.processKeyDown(event.key.keysym); break;
        case SDL_KEYUP: camera.processKeyUp(event.key.keysym); break;
        case SDL_MOUSEMOTION: camera.processMouseMotion(event.motion); break;
        case SDL_MOUSEBUTTONDOWN: camera.processMouseButtonDown(event.button); break;
        case SDL_MOUSEBUTTONUP: camera.processMouseButtonUp(event.button); break;
        case SDL_MOUSEWHEEL: camera.processMouseWheel(event.wheel); break;

        case SDL_DROPFILE: {
            droppedFilePath = event.drop.file;
            std::filesystem::path filePath(droppedFilePath);

            if (filePath.extension().string() == ".fbx") {
                meshes.clear();
                textureID = 0;

                meshes = importer.loadFBX(droppedFilePath, textureID);

                for (size_t i = 0; i < meshes.size(); ++i) {
                    const std::string objectName = getFileName(droppedFilePath) + "_" + std::to_string(i);
                    GameObject* modelObject = new GameObject(objectName, meshes[i], textureID);
                    variables->window->gameObjects.push_back(modelObject);
                }

                std::string outputPath = "Assets/" + getFileName(droppedFilePath) + ".dat";
                importer.saveCustomFormat(outputPath, meshes);

                console.addLog("FBX loaded & saved in: " + outputPath);
               /* std::cout << "FBX loaded & saved in: " << outputPath << std::endl;*/

                break; 
            }
            else if (filePath.extension().string() == ".png" || filePath.extension().string() == ".jpg" || filePath.extension().string() == ".dds") {
                console.addLog("PNG texture dropped: " + std::string(droppedFilePath));
              /*  std::cout << "PNG texture dropped: " << droppedFilePath << std::endl;*/
                variables->textureFilePath = filePath.string();
                if (variables->window->selectedObject) {
                    GLuint newTextureID = importer.loadTexture(droppedFilePath); 
                    variables->window->selectedObject->textureID = newTextureID; 
                    console.addLog("Texture applied to selected object.");
                /*    std::cout << "Texture applied to selected object." << std::endl; */
                }
                else {
                    console.addLog("No object selected to apply the texture.");
                /*    std::cout << "No object selected to apply the texture." << std::endl; */
                }
                SDL_free(event.drop.file);
                break;
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

void drawGrid(float spacing) {
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

void render(const std::vector<GameObject*>& gameObjects) {
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
   
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, Variables::WINDOW_SIZE.x, Variables::WINDOW_SIZE.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glFlush();
}

void cleanupFrameBuffer() {
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &textureColorbuffer);
    glDeleteRenderbuffers(1, &rbo);
}
