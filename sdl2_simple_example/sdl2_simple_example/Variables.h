#pragma once
#include "MyWindow.h"
#include <glm/glm.hpp>

class Variables {
public:
	MyWindow* window;

	static const glm::ivec2 WINDOW_SIZE;
}; 

extern Variables* variables;