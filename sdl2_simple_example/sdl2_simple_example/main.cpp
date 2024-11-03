#include <GL/glew.h>
#include <chrono>
#include <thread>
#include <exception>
#include <glm/glm.hpp>
#include <SDL2/SDL_events.h>
#include <iostream>
#include "MyWindow.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "assimp/Importer.hpp"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "IL/il.h"
#include "IL/ilu.h"
#include "IL/ilut.h"
#include "Importer.h"
#include <filesystem>
#include "Camera.h"
#include "GameObject.h"

#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

using namespace std;
using namespace chrono;
namespace fs = filesystem;

using hrclock = high_resolution_clock;
using u8vec4 = glm::u8vec4;
using ivec2 = glm::ivec2;
using vec3 = glm::dvec3;

static const ivec2 WINDOW_SIZE(1024, 1024);
static const unsigned int FPS = 60;
static const auto FRAME_DT = 1.0s / FPS;

Camera camera; 

// Drop Objects
vector<MeshData> meshes;
GLuint textureID;
Importer importer;
const char* fbxFile = nullptr;

vector<GameObject> gameObjects;

static void init_openGL() {
	glewInit();
	if (!GLEW_VERSION_3_0) throw exception("OpenGL 3.0 API is not available.");
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glClearColor(0.5, 0.5, 0.5, 1.0);
}

string getFileName(const string& path) {
	return fs::path(path).stem().string();
}

static bool processEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			return false;
			break;

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

			for (size_t i = 0; i < meshes.size(); ++i)
			{
				string objectName = getFileName(fbxFile) + "_" + to_string(i);
				gameObjects.emplace_back(objectName, meshes[i], textureID);
			}

			string baseName = getFileName(fbxFile);
			string outputPath = "Assets/" + baseName + ".dat";
			importer.saveCustomFormat(outputPath, meshes);

			cout << "Archivo FBX cargado y guardado en: " << outputPath << endl;
			break;
		}
		default:
			ImGui_ImplSDL2_ProcessEvent(&event);
			break;
		}
	}

	camera.move(SDL_GetKeyboardState(NULL));
	return true;
}


void render(const vector<GameObject>& gameObjects)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, 1.0f, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	camera.applyCameraTransformations();

	for (const auto& obj : gameObjects)
	{	
		glPushMatrix();

		glm::mat4 transform = obj.getTransformMatrix();
		glMultMatrixf(glm::value_ptr(transform));

		if (obj.textureID) {
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, obj.textureID);
		}
		else {
			glDisable(GL_TEXTURE_2D);
		}

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, obj.meshData.vertices.data());

		if (!obj.meshData.textCoords.empty()) {
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, 0, obj.meshData.textCoords.data());
		}

		glDrawElements(GL_TRIANGLES, obj.meshData.indices.size(), GL_UNSIGNED_INT, obj.meshData.indices.data());

		glDisableClientState(GL_VERTEX_ARRAY);

		if (!obj.meshData.textCoords.empty()) {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}

		glPopMatrix();
	}
	glFlush();
}

int main(int argc, char** argv) {
	MyWindow window("SDL2 Simple Example", WINDOW_SIZE.x, WINDOW_SIZE.y);
	init_openGL();

	meshes = importer.loadFBX("Assets/BakerHouse.fbx", textureID);

	while (processEvents()) {
		const auto t0 = hrclock::now();

		render(gameObjects);
		window.swapBuffers();

		const auto t1 = hrclock::now();
		const auto dt = t1 - t0;
		if (dt < FRAME_DT) this_thread::sleep_for(FRAME_DT - dt);
	}
	return 0;
}