#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <string>
#include <vector>
#include "GameObject.h"
#include "ConsoleWindow.h"

struct SDL_Window;

class MyWindow {

	void* _ctx = nullptr;

	int _width = 0;
	int _height = 0;
public:

	SDL_Window* _window = nullptr;
	std::vector<GameObject*> gameObjects;
	std::vector<GameObject*> selectedObjects;
	GameObject* selectedObject;

public:
	int width() const { return _width; }
	int height() const { return _height; }
	double aspectRatio() const { return static_cast<double>(_width) / _height; }

	MyWindow(const std::string& title, int w, int h);
	~MyWindow();

	SDL_Window* getSDLWindow() const { return _window; }

	void swapBuffers();
	std::vector<GameObject*> getGameObjects() { return gameObjects; }

	void selectObject(GameObject* obj);

	void createControlWindow();

	void configMyWindow();

	void createDockSpace();
	void createMainMenu();

	/*template <class Archive>
	void serialize(Archive& archive) {
		archive(CEREAL_NVP(_width), CEREAL_NVP(_height), CEREAL_NVP(_fps), CEREAL_NVP(_frameCount));
		archive(CEREAL_NVP(gameObjects));
		archive(CEREAL_NVP(selectedObject), CEREAL_NVP(objectSelected));
	}*/

private:
	unsigned int _lastTime = 0;
	unsigned int _currentTime = 0;
	unsigned int _fps = 0;
	unsigned int _frameCount = 0;
	std::vector<float> _fpsHistory;

	bool showFiles = false;        // Para saber si mostrar los archivos
	std::string selectedFile;      // Para almacenar el archivo seleccionado
	std::string folderPath = "Assets";

};

#endif // MYWINDOW_H	