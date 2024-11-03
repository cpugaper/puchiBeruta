#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <string>
#include <vector>
#include "GameObject.h"

struct SDL_Window;

class MyWindow {

	SDL_Window* _window = nullptr;
	void* _ctx = nullptr;

	int _width = 0;
	int _height = 0;

	std::vector<GameObject> gameObjects;

public:
	int width() const { return _width; }
	int height() const { return _height; }
	double aspectRatio() const { return static_cast<double>(_width) / _height; }

	MyWindow(const std::string& title, int w, int h);
	~MyWindow();

	void swapBuffers();
	std::vector<GameObject>& getGameObjects() { return gameObjects; }
};
#endif // MYWINDOW_H	