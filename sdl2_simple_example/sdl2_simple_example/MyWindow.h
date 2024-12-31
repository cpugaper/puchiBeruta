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

	void configMyWindow();

	void createDockSpace();
	void createMainMenu();

private:
	unsigned int _lastTime = 0;
	unsigned int _currentTime = 0;
	unsigned int _fps = 0;
	unsigned int _frameCount = 0;
	std::vector<float> _fpsHistory;

	bool showFiles = false; 
	std::string selectedFile;
	std::string folderPath = "Assets";
};

#endif // MYWINDOW_H	