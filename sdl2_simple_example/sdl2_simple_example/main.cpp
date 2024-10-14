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
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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
	//draw_cube(vec3(0.0, 0.0, 0.0), 0.3);
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
struct Mesh
{
	vector<GLfloat> vertices;
	vector<GLuint> indices;
};

vector<Mesh> meshes;

void loadFBX(const char* filePath) {
	const struct aiScene* scene = aiImportFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!scene) {
		fprintf(stderr, "Error al cargar el fichero: %s\n", aiGetErrorString());
	}

	// Procesar las mallas
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		const aiMesh* mesh = scene->mMeshes[i];

		/*Mesh mesh;*/

		//// Cargar los v�rtices
		//for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
		//	aiVector3D vertex = mesh->mVertices[v];
		//	mesh->vertices.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
		//}

		//// Cargar los �ndices
		//for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
		//	aiFace face = mesh->mFaces[f];
		//	for (unsigned int j = 0; j < face.mNumIndices; j++) {
		//		mesh.indices.push_back(face.mIndices[j]);
		//	}
		//}
	}

	aiReleaseImport(scene);
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
	
	const char* file = "C:/Users/rebecafl/Documents/GitHub/puchiBeruta/Assets/cube.fbx"; // Ruta del fitxer a carregar
	const struct aiScene* scene = aiImportFile(file, aiProcess_Triangulate);
	loadFBX(file); 
	if (!scene) {
		fprintf(stderr, "Error en carregar el fitxer: %s\n", aiGetErrorString());
		return -1;
	}
	printf("Numero de malles: %u\n", scene->mNumMeshes);

	//for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
	//	aiMesh* mesh = scene->mMeshes[i];
	//	printf("\nMalla %u:\n", i);
	//	printf(" Numero de vertexs: %u\n", mesh->mNumVertices);
	//	printf(" Numero de triangles: %u\n", mesh->mNumFaces);
	//	// V�rtexs
	//	for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
	//		aiVector3D vertex = mesh->mVertices[v];
	//		printf(" Vertex %u: (%f, %f, %f)\n", v, vertex.x, vertex.y, vertex.z);
	//	}
	//	// �ndexs de triangles (3 per triangle)
	//	for (unsigned int f = 0; f < mesh->mNumFaces; f++) {

	//		aiFace face = mesh->mFaces[f];
	//		printf(" Indexs triangle %u: ", f);

	//		for (unsigned int j = 0; j < face.mNumIndices; j++) {
	//			printf("%u ", face.mIndices[j]);
	//		}

	//		printf("\n");
	//	}
	//}
	aiReleaseImport(scene);

	return 0;
}
