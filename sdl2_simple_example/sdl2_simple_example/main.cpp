#include <iostream>
#include <filesystem>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <chrono>
#include <thread>
#include <exception>

// CUSTOM
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
#include "Camera.h"
#include "GameObject.h"
#include "Renderer.h"
#include "Variables.h"

#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

using namespace std::chrono;

using hrclock = high_resolution_clock;
using ivec2 = glm::ivec2;

static const ivec2 WINDOW_SIZE(1024, 1024);
static const unsigned int FPS = 60;
static const auto FRAME_DT = 1.0s / FPS;

Camera camera;

// Drop Objects
std::vector<MeshData> meshes;
GLuint textureID;
Importer importer;
const char* fbxFile = nullptr;

std::vector<GameObject> gameObjects;

#undef main
int main(int argc, char** argv) {
	variables = new Variables; 
    variables->window = new MyWindow("SDL2 Simple Example", WINDOW_SIZE.x, WINDOW_SIZE.y);
	initOpenGL();

	meshes = importer.loadFBX("Assets/BakerHouse.fbx", textureID);
	for (size_t i = 0; i < meshes.size(); ++i) {
		std::string objectName = getFileName("Assets/BakerHouse.fbx") + "_" + std::to_string(i);
		//gameObjects.emplace_back(objectName, meshes[i], textureID);
		GameObject* casa = new GameObject(objectName, meshes[i], textureID);
		variables->window->gameObjects.push_back(casa);
	}

	while (processEvents(camera, gameObjects, fbxFile)) {
		const auto t0 = hrclock::now();

		render(variables->window->gameObjects); 
		variables->window->swapBuffers();  

		const auto t1 = hrclock::now();
		const auto dt = t1 - t0;
		if (dt < FRAME_DT) std::this_thread::sleep_for(FRAME_DT - dt);
	}

	for (const auto& obj : gameObjects) {
		std::cout << "Objeto en la escena: " << obj.getName() << std::endl;
	}

	return 0;
}