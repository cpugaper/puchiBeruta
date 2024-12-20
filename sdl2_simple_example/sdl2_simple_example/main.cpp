#include <iostream>
#include <filesystem>
#include <windows.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <chrono>
#include <thread>
#include <exception>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "assimp/Importer.hpp"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "IL/ilut.h"
#include "Importer.h"
#include "MyWindow.h"
#include "Camera.h"
#include "GameObject.h"
#include "Renderer.h"
#include "Variables.h"
#include "ConsoleWindow.h"

#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

using namespace std::chrono;

using hrclock = high_resolution_clock;
static const unsigned int FPS = 60;
static const auto FRAME_DT = 1.0s / FPS;

extern Camera camera;
extern Renderer renderer;
std::vector<MeshData> meshes;
GLuint textureID;
extern Importer importer;
extern std::vector<GameObject> gameObjects;
const char* fbxFile = nullptr;

#undef main
int main(int argc, char** argv) {

	variables = new Variables;

	console.addLog("Initializing SDL...");
	variables->window = new MyWindow("SDL2 Simple Example", Variables::WINDOW_SIZE.x, Variables::WINDOW_SIZE.y);
	console.addLog("SDL initialized with success");

	console.addLog("Initializing OpenGL...");
	renderer.initOpenGL();
	console.addLog("OpenGL initialized with success");

	console.addLog("Initializing Devil...");
	ilInit();
	iluInit();
	console.addLog("DevIL initialized with success");

	//std::string sceneFile = "scene.json";
	//if (std::filesystem::exists(sceneFile)) {
	//	std::cout << "Loading scene from file: " << sceneFile << std::endl;
	//	meshes = importer.loadScene(sceneFile); 
	//}
	//else {
	//	std::cout << "Loading model FBX..." << std::endl;
	//	meshes = importer.loadFBX("Assets/BakerHouse.fbx", textureID);

	//	std::cout << "Savign scene in file: " << sceneFile << std::endl;
	//	importer.saveScene(sceneFile, meshes);
	//}

	meshes = importer.loadModelFromCustomFormat("sdl2_simple_example\\Library\\Models\\BakerHouse.dat", textureID);

	for (size_t i = 0; i < meshes.size(); ++i) {
		std::string objectName = renderer.getFileName("sdl2_simple_example\\Library\\Models\\BakerHouse.dat") + "_" + std::to_string(i);
		auto casa = new GameObject(objectName, meshes[i], 0);
		variables->window->gameObjects.push_back(casa);
	}

	// Main loop: handling events, rendering and maintaining FPS
	while (renderer.processEvents(camera, gameObjects, fbxFile)) {
		const auto t0 = hrclock::now();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		variables->window->createDockSpace();
		renderer.render(variables->window->gameObjects);

		ImGui::Render();
		ImGui::UpdatePlatformWindows();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		variables->window->swapBuffers();

		const auto t1 = hrclock::now();
		const auto dt = t1 - t0;
		if (dt < FRAME_DT) std::this_thread::sleep_for(FRAME_DT - dt);
	}

	for (const auto& obj : gameObjects) {
		console.addLog("Objeto en la escena: " + obj.getName());
	}

	renderer.cleanupFrameBuffer();

	return 0;
}