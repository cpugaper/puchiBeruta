#ifndef PROJECTWINDOW_H

#include <string>
#include <vector>
#include <filesystem>
#include "GameObject.h"

class ProjectWindow {
public:
    void render();

private:
    std::string selectedFilePath;
};

#endif // PROJECTWINDOW_H
