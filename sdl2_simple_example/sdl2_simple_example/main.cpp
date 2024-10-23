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
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

using namespace std;

using hrclock = chrono::high_resolution_clock;
using u8vec4 = glm::u8vec4;
using ivec2 = glm::ivec2;
using vec3 = glm::dvec3;

static const ivec2 WINDOW_SIZE(512, 512);
static const unsigned int FPS = 60;
static const auto FRAME_DT = 1.0s / FPS;

GLdouble cameraX = 0.0f;
GLdouble cameraY = 0.0f;
GLdouble cameraZ = -10.0f;
float cameraAngleY = 0.0f;
float cameraAngleX = 0.0f;

float rotationAngle = 0.0f;

static void init_openGL() {
	glewInit();
	if (!GLEW_VERSION_3_0) throw exception("OpenGL 3.0 API is not available.");
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
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
//
//static void display_func() {
// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// //draw_triangle(u8vec4(255, 0, 0, 255), vec3(0.0, 0.0, 0.0), 0.5);
// //draw_cube(vec3(0.0, 0.0, 0.0), 0.3);
// render();
//}

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
	vector<GLfloat> textCoords;
};

vector<Mesh> meshes;

void loadFBX(const char* filePath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate);

	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* aimesh = scene->mMeshes[i];

		Mesh mesh;

		for (unsigned int v = 0; v < aimesh->mNumVertices; v++)
		{
			mesh.vertices.push_back(aimesh->mVertices[v].x);
			mesh.vertices.push_back(aimesh->mVertices[v].y);
			mesh.vertices.push_back(aimesh->mVertices[v].z);

			mesh.textCoords.push_back(aimesh->mTextureCoords[0][v].x);
			mesh.textCoords.push_back(aimesh->mTextureCoords[0][v].y);
		}

		for (unsigned int f = 0; f < aimesh->mNumFaces; f++)
		{
			const aiFace& face = aimesh->mFaces[f];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				mesh.indices.push_back(face.mIndices[j]);
			}
		}
		meshes.push_back(mesh);

		/*if (aimesh->mTextureCoords[0])
		{
			for (unsigned int v = 0; v < aimesh->mNumVertices; v++)
			{
			}
		}*/
	}
}

GLuint loadTexture(const char* filename)
{
	ILuint imageID;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);


	bool ok = ilLoadImage((const wchar_t *)filename);

	if (!ok) {
	
		printf("AAAAAAHHH!!!");
	}

	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
		ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE,
		ilGetData());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	ilDeleteImages(1, &imageID);

	return textureID;
}

GLuint TextureID;

void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, 1.0f, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cameraX, cameraY, cameraZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	glRotatef(cameraAngleY, 1.0f, 0.0f, 0.0f);
	glRotatef(cameraAngleX, 0.0f, 1.0f, 0.0f);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, TextureID);

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);
	glRotatef(rotationAngle, 1.0f, 1.0f, 0.0f);


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

	glPopMatrix();
	glFlush();
}

int main(int argc, char** argv) {
	MyWindow window("SDL2 Simple Example", WINDOW_SIZE.x, WINDOW_SIZE.y);

	init_openGL();

	ilInit();
	iluInit();
	ilutInit();
	ilutRenderer(ILUT_OPENGL);

	const char* file = "C:/Users/rebecafl/Documents/GitHub/puchiBeruta/Assets/BakerHouse.fbx";
	loadFBX(file);

	TextureID = loadTexture("C:/Users/rebecafl/Documents/GitHub/puchiBeruta/Assets/Baker_House.png");

	if (TextureID == 0)
	{
		std::cerr << "Error: No se pudo cargar la textura." << std::endl;
		return -1;
	}


	while (processEvents()) {
		const auto t0 = hrclock::now();
		//display_func();
		rotationAngle += 0.5f;
		render();
		window.swapBuffers();
		const auto t1 = hrclock::now();
		const auto dt = t1 - t0;
		if (dt < FRAME_DT) this_thread::sleep_for(FRAME_DT - dt);
	}

	const struct aiScene* scene = aiImportFile(file, aiProcess_Triangulate);

	if (!scene) {
		fprintf(stderr, "Error en carregar el fitxer: %s\n", aiGetErrorString());
		return -1;
	}
	printf("Numero de malles: %u\n", scene->mNumMeshes);

	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];
		printf("\nMalla %u:\n", i);
		printf(" Numero de vertexs: %u\n", mesh->mNumVertices);
		printf(" Numero de triangles: %u\n", mesh->mNumFaces);

		for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
			aiVector3D vertex = mesh->mVertices[v];
			printf(" Vertex %u: (%f, %f, %f)\n", v, vertex.x, vertex.y, vertex.z);
		}

		for (unsigned int f = 0; f < mesh->mNumFaces; f++) {

			aiFace face = mesh->mFaces[f];
			printf(" Indexs triangle %u: ", f);

			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				printf("%u ", face.mIndices[j]);
			}
			printf("\n");
		}
	}
	aiReleaseImport(scene);

	return 0;
}