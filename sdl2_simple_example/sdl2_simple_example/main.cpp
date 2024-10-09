#include <GL/glew.h>
#include <chrono>
#include <thread>
#include <exception>
#include <glm/glm.hpp>
#include <SDL2/SDL_events.h>
#include "MyWindow.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
using namespace std;

using hrclock = chrono::high_resolution_clock;
using u8vec4 = glm::u8vec4;
using ivec2 = glm::ivec2;
using vec3 = glm::dvec3;

static const ivec2 WINDOW_SIZE(512, 512);
static const unsigned int FPS = 60;
static const auto FRAME_DT = 1.0s / FPS;

static void init_openGL() {
	glewInit();
	if (!GLEW_VERSION_3_0) throw exception("OpenGL 3.0 API is not available.");
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.5, 0.5, 0.5, 1.0);
}

static void draw_triangle(const u8vec4& color, const vec3& center, double size) {
	glColor4ub(color.r, color.g, color.b, color.a);
	glBegin(GL_TRIANGLES);
	glVertex3d(center.x, center.y + size, center.z);
	glVertex3d(center.x - size, center.y - size, center.z);
	glVertex3d(center.x + size, center.y - size, center.z);
	glEnd();

}

static void draw_cube(const vec3& center, double size)
{
	//CARA frontal
	glColor4ub(176, 196, 222, 255);
	glBegin(GL_TRIANGLES);
	glVertex3d(center.x - size, center.y - size, center.z); // V1
	glVertex3d(center.x + size, center.y - size, center.z); // V2
	glVertex3d(center.x - size, center.y + size, center.z); // V3
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3d(center.x + size, center.y - size, center.z); // V2
	glVertex3d(center.x + size, center.y + size, center.z); // V4
	glVertex3d(center.x - size, center.y + size, center.z); // V3
	glEnd();

	////CARA trasera
	glColor4ub(176, 224, 230, 255);
	glBegin(GL_TRIANGLES);
	glVertex3d(center.x - size, center.y - size, center.z - size * 2); // V5
	glVertex3d(center.x + size, center.y - size, center.z - size * 2); // V6
	glVertex3d(center.x - size, center.y + size, center.z - size * 2); // V7
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3d(center.x + size, center.y - size, center.z - size * 2); // V6
	glVertex3d(center.x + size, center.y + size, center.z - size * 2); // V8
	glVertex3d(center.x - size, center.y + size, center.z - size * 2); // V7
	glEnd();

	////CARA derecha
	glColor4ub(173, 216, 230, 255);
	glBegin(GL_TRIANGLES);
	glVertex3d(center.x + size, center.y + size, center.z); // V4
	glVertex3d(center.x + size, center.y - size, center.z); // V2
	glVertex3d(center.x + size, center.y - size, center.z - size * 2); // V6
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3d(center.x + size, center.y + size, center.z - size * 2); // V8
	glVertex3d(center.x + size, center.y + size, center.z); // V4
	glVertex3d(center.x + size, center.y - size, center.z - size * 2); // V6
	glEnd();

	////CARA izquierda
	glColor4ub(135, 206, 235, 255);
	glBegin(GL_TRIANGLES);
	glVertex3d(center.x - size, center.y + size, center.z); // V3
	glVertex3d(center.x - size, center.y - size, center.z - size * 2); // V5
	glVertex3d(center.x - size, center.y - size, center.z); // V1
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3d(center.x - size, center.y + size, center.z); // V3
	glVertex3d(center.x - size, center.y - size, center.z - size * 2); // V5
	glVertex3d(center.x - size, center.y + size, center.z - size * 2); // V7
	glEnd();

	////CARA superior
	glColor4ub(135, 206, 250, 255);
	glBegin(GL_TRIANGLES);
	glVertex3d(center.x - size, center.y + size, center.z); // V3
	glVertex3d(center.x - size, center.y + size, center.z - size * 2); // V7
	glVertex3d(center.x + size, center.y + size, center.z); // V4
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3d(center.x - size, center.y + size, center.z - size * 2); // V7
	glVertex3d(center.x + size, center.y + size, center.z); // V4
	glVertex3d(center.x + size, center.y + size, center.z - size * 2); // V8
	glEnd();

	////CARA inferior
	glColor4ub(0, 191, 255, 255);
	glBegin(GL_TRIANGLES);
	glVertex3d(center.x - size, center.y - size, center.z - size * 2); // V5
	glVertex3d(center.x - size, center.y - size, center.z); // V1
	glVertex3d(center.x + size, center.y - size, center.z); // V2
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3d(center.x - size, center.y - size, center.z - size * 2); // V5
	glVertex3d(center.x + size, center.y - size, center.z - size * 2); // V6
	glVertex3d(center.x + size, center.y - size, center.z); // V2
	glEnd();


	glRotatef(0.5f, 1.0f, 1.0f, 0.0f);
}

static void display_func() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//draw_triangle(u8vec4(255, 0, 0, 255), vec3(0.0, 0.0, 0.0), 0.5);
	draw_cube(vec3(0.0, 0.0, 0.0), 0.3);
}

static bool processEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			return false;
			break;
		default:
			ImGui_ImplSDL2_ProcessEvent(&event);
			break;
		}
	}
	return true;
}

int main(int argc, char** argv) {
	MyWindow window("SDL2 Simple Example", WINDOW_SIZE.x, WINDOW_SIZE.y);

	init_openGL();

	while (processEvents()) {
		const auto t0 = hrclock::now();
		display_func();
		window.swapBuffers();
		const auto t1 = hrclock::now();
		const auto dt = t1 - t0;
		if(dt<FRAME_DT) this_thread::sleep_for(FRAME_DT - dt);
	}

	return 0;
}