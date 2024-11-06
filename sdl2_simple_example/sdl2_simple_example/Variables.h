#pragma once
#include "MyWindow.h"
#include <glm/glm.hpp>

class Variables {
public:
	MyWindow* window;

	static const glm::ivec2 WINDOW_SIZE;

	// Variables de configuraci�n (las inicializas con valores predeterminados)
	int windowWidth = 1280;
	int windowHeight = 800;
	bool fullscreen = false;
	bool vsyncEnabled = true;  // Activar/desactivar V-Sync

	// Variables de configuraci�n para texturas
	float textureFilterQuality = 1.0f;  // Usar un filtro simple para las texturas
	float textureAnisotropicLevel = 4.0f;  // Nivel de anisotrop�a para las texturas

};

extern Variables* variables;