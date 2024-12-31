#ifndef ASSETSWINDOW_H

#include <string>
#include <vector>
#include <filesystem>
#include "GameObject.h"

class AssetsWindow {
public:
    void render();

private:
    std::string selectedFilePath;
};

#endif // ASSETSWINDOW_H