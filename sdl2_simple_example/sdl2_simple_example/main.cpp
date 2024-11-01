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

#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

using namespace std;
using namespace chrono;
namespace fs = filesystem;

using hrclock = high_resolution_clock;
using u8vec4 = glm::u8vec4;
using ivec2 = glm::ivec2;
using vec3 = glm::dvec3;

static const ivec2 WINDOW_SIZE(720, 720);
static const unsigned int FPS = 60;
static const auto FRAME_DT = 1.0s / FPS;

GLdouble cameraX = 0.0f, cameraY = 0.0f, cameraZ = -10.0f;
float cameraAngleY = 0.0f, cameraAngleX = 0.0f;
bool isAltPressed = false, isLeftMouseDragging = false, isRightMouseDragging = false;
int lastMouseX, lastMouseY;
float rotationAngle = 0.0f;

// Arrastre de Objetos
vector<MeshData> meshes;
GLuint textureID;
Importer importer;
const char* fbxFile = nullptr;

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

			//DETECTA SI PRESIONAMOS LA TECLA ALT
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_LALT || event.key.keysym.sym == SDLK_RALT) {
				isAltPressed = true;
			}
			break;

		case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_LALT || event.key.keysym.sym == SDLK_RALT) {
				isAltPressed = false;
			}
			break;

		case SDL_MOUSEMOTION:
			if (isLeftMouseDragging) {
				int mouseX = event.motion.x;
				int mouseY = event.motion.y;
				int deltaX = mouseX - lastMouseX;
				int deltaY = mouseY - lastMouseY;

				if (isAltPressed) {
					cameraAngleX += deltaX * 0.1f;
					cameraAngleY += deltaY * 0.1f;
				}
				lastMouseX = mouseX;
				lastMouseY = mouseY;
			}
			else if (isRightMouseDragging) {
				int mouseX = event.motion.x;
				int mouseY = event.motion.y;
				int deltaX = mouseX - lastMouseX;
				int deltaY = mouseY - lastMouseY;

				if (isAltPressed) {
					cameraX += deltaX * 0.02f;
					cameraY -= deltaY * 0.02f;
				}

				lastMouseX = mouseX;
				lastMouseY = mouseY;
			}
			break;

			// DETECTA SI HACEMOS CLICK IZQUIERDO
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_LEFT && isAltPressed) {
				isLeftMouseDragging = true;
				lastMouseX = event.button.x;
				lastMouseY = event.button.y;
			}
			else if (event.button.button == SDL_BUTTON_RIGHT && isAltPressed) {
				isRightMouseDragging = true;
				lastMouseX = event.button.x;
				lastMouseY = event.button.y;
			}
			break;

		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_LEFT) {
				isLeftMouseDragging = false;
			}
			else if (event.button.button == SDL_BUTTON_RIGHT) {
				isRightMouseDragging = false;
			}
			break;

		case SDL_MOUSEWHEEL:
			if (isAltPressed) {
				cameraZ += event.wheel.y * 0.3f;
			}
			break;
		case SDL_DROPFILE: {
			fbxFile = event.drop.file;
			meshes.clear();
			textureID = 0;

			meshes = importer.loadFBX(fbxFile, textureID);

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
	return true;
}


void render(const vector<MeshData>& meshes, GLuint textureID)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, 1.0f, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(cameraX, cameraY, cameraZ);
	glRotatef(cameraAngleY, 1.0f, 0.0f, 0.0f);
	glRotatef(cameraAngleX, 0.0f, 1.0f, 0.0f);

	if (textureID) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textureID);
	}
	else {
		glDisable(GL_TEXTURE_2D);
	}

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);

	for (const auto& mesh : meshes)
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, mesh.vertices.data());

		if (!mesh.textCoords.empty()) {
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, 0, mesh.textCoords.data());
		}

		glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, mesh.indices.data());

		glDisableClientState(GL_VERTEX_ARRAY);

		if (!mesh.textCoords.empty()) {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	}
	glFlush();
}

int main(int argc, char** argv) {
	MyWindow window("SDL2 Simple Example", WINDOW_SIZE.x, WINDOW_SIZE.y);
	init_openGL();

	while (processEvents()) {
		const auto t0 = hrclock::now();
		rotationAngle += 0.5f;
		render(meshes, textureID);
		window.swapBuffers();
		const auto t1 = hrclock::now();
		const auto dt = t1 - t0;
		if (dt < FRAME_DT) this_thread::sleep_for(FRAME_DT - dt);
	}

	return 0;
}