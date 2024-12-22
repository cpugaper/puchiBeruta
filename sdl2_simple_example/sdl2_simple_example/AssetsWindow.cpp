#include "AssetsWindow.h"
#include "ConsoleWindow.h"
#include "Renderer.h"
#include "imgui.h"
#include <iostream>
#include <map>
#include <Windows.h>
#include <shellapi.h>
#include <commctrl.h>

extern Renderer renderer;
extern ConsoleWindow console;

namespace {
    struct IconCache {
        HICON hIcon;
        ImTextureID textureID;

        IconCache() : hIcon(nullptr), textureID(nullptr) {}
    };

    std::map<std::string, IconCache> iconCache;
    std::map<std::string, std::string> customIconPaths;

    IconCache getFileIcon(const std::string& filePath, bool isDirectory) {
        auto it = iconCache.find(filePath);
        if (it != iconCache.end()) {
            return it->second;
        }

        IconCache cache;
        SHFILEINFOA shFileInfo;

        if (isDirectory) {
            cache.hIcon = (HICON)LoadImageA(NULL, "\\Icons\\folder.ico",
                IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
        }
        else {
            std::string ext = std::filesystem::path(filePath).extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            auto customIt = customIconPaths.find(ext);
            if (customIt != customIconPaths.end()) {
                cache.hIcon = (HICON)LoadImageA(NULL, customIt->second.c_str(),
                    IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
            }
            else {
                SHGetFileInfoA(filePath.c_str(), 0, &shFileInfo, sizeof(shFileInfo),
                    SHGFI_ICON | SHGFI_SMALLICON);
                cache.hIcon = shFileInfo.hIcon;
            }
        }

        if (cache.hIcon) {
            ICONINFO iconInfo;
            GetIconInfo(cache.hIcon, &iconInfo);

            HDC dc = GetDC(NULL);
            HDC memDC = CreateCompatibleDC(dc);
            BYTE* pixels = new BYTE[16 * 16 * 4];

            BITMAPINFO bmi = { 0 };
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = 16;
            bmi.bmiHeader.biHeight = -16;
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            GetDIBits(memDC, iconInfo.hbmColor, 0, 16, pixels, &bmi, DIB_RGB_COLORS);

            GLuint textureID;
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            cache.textureID = (ImTextureID)(intptr_t)textureID;

            delete[] pixels;
            DeleteDC(memDC);
            ReleaseDC(NULL, dc);
            DeleteObject(iconInfo.hbmColor);
            DeleteObject(iconInfo.hbmMask);
        }

        iconCache[filePath] = cache;
        return cache;
    }

    void setCustomIconForExtension(const std::string& extension, const std::string& iconPath) {
        customIconPaths[extension] = iconPath;
    }

    void handleFileDeletion(const std::string& filePath, std::string& selectedFilePath) {
        try {
            std::filesystem::remove(filePath);
            console.addLog(("Deleted file: " + filePath).c_str());
            if (selectedFilePath == filePath) {
                selectedFilePath.clear();
            }

            auto it = iconCache.find(filePath);
            if (it != iconCache.end()) {
                if (it->second.hIcon) DestroyIcon(it->second.hIcon);
                if (it->second.textureID) glDeleteTextures(1, (GLuint*)&it->second.textureID);
                iconCache.erase(it);
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            console.addLog(("Error deleting file: " + std::string(e.what())).c_str());
        }
    }

    void renderFileContextMenu(const std::string& filePath, std::string& selectedFilePath) {
        if (ImGui::BeginPopupContextItem()) {
            float initialX = ImGui::GetCursorPosX();
            ImGui::SetCursorPosX(initialX + 20.0f);
            if (ImGui::Selectable("Delete")) {
                handleFileDeletion(filePath, selectedFilePath);
            }
            ImGui::EndPopup();
        }
    }

    void renderFileDragDrop(const std::string& filePath, const std::string& fileName) {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ImGui::SetDragDropPayload("LibraryFile", filePath.c_str(), filePath.length() + 1);
            ImGui::Text("Drag: %s", fileName.c_str());
            ImGui::EndDragDropSource();
        }
    }

    void renderRegularFile(const std::filesystem::directory_entry& entry, std::string& selectedFilePath) {
        std::string filePath = entry.path().string();
        std::string fileName = entry.path().filename().string();
        std::string extension = entry.path().extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        IconCache icon = getFileIcon(filePath, false);

        ImGui::PushID(filePath.c_str());

        // Renderizar el icono
        if (icon.textureID) {
            ImGui::Image(icon.textureID, ImVec2(20, 20));
            ImGui::SameLine();
        }

        if (ImGui::Selectable(fileName.c_str(), selectedFilePath == filePath)) {
            selectedFilePath = filePath;
        }

        renderFileContextMenu(filePath, selectedFilePath);
        renderFileDragDrop(filePath, fileName);
        ImGui::PopID();
    }
}

void AssetsWindow::render() {
    ImGui::Begin("Assets", nullptr);
    static std::string libraryPath = "Library";
    static std::string selectedFilePath;
    static std::map<std::string, bool> folderExpanded;

    static bool initialized = false;
    if (!initialized) {
        setCustomIconForExtension(".dat", "Icons\\object.ico");
        setCustomIconForExtension(".png", "Icons\\image.ico");
        initialized = true;
    }

    if (!std::filesystem::exists(libraryPath)) {
        ImGui::Text("Library folder not found: %s", libraryPath.c_str());
        ImGui::End();
        return;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)) && !selectedFilePath.empty()) {
        handleFileDeletion(selectedFilePath, selectedFilePath);
    }

    for (const auto& entry : std::filesystem::directory_iterator(libraryPath)) {
        if (entry.is_directory()) {
            std::string filePath = entry.path().string();
            std::string fileName = entry.path().filename().string();

            IconCache icon = getFileIcon(filePath, true);

            ImGui::PushID(filePath.c_str());

            if (ImGui::TreeNode(fileName.c_str())) {
                if (icon.textureID) {
                    ImGui::Image(icon.textureID, ImVec2(15, 15));
                    ImGui::SameLine();
                }
                folderExpanded[filePath] = true;
                for (const auto& subEntry : std::filesystem::directory_iterator(filePath)) {
                    if (subEntry.is_regular_file()) {
                        renderRegularFile(subEntry, selectedFilePath);
                    }
                }
                ImGui::TreePop();
            }
            else {
                folderExpanded[filePath] = false;
            }
            ImGui::PopID();
        }
        else if (entry.is_regular_file()) {
            renderRegularFile(entry, selectedFilePath);
        }
    }
    ImGui::End();
}