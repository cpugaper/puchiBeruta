#include "MyWindow.h"
#include <glm/glm.hpp>
#include <string>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/memory.hpp>  
#include <cereal/archives/json.hpp>

// Global and window variables
class Variables {
public:
	MyWindow* window;

	static const glm::ivec2 WINDOW_SIZE;

	int windowWidth = 1280;
	int windowHeight = 800;
	bool fullscreen = false;
	bool vsyncEnabled = true;
	float textureFilterQuality = 1.0f;
	float textureAnisotropicLevel = 4.0f;

	int texturewidth = 0, textureheight = 0;

	std::string textureFilePath;
	std::string checkerTexture = "Assets/checker_texture.png";
};

extern Variables* variables;