#include "ProjectWindow.h"
#include "ConsoleWindow.h"
#include "Renderer.h"
#include "imgui.h"
#include <iostream>
#include <map>

extern Renderer renderer;
extern ConsoleWindow console;

void ProjectWindow::render() {
    ImGui::Begin("Project", nullptr);

    static std::string libraryPath = "Library";
    static std::string selectedFilePath;
    static std::map<std::string, bool> folderExpanded;

    if (!std::filesystem::exists(libraryPath)) {
        ImGui::Text("Assets folder not found: %s", libraryPath.c_str());
        ImGui::End();
        return;
    }

    // Check for delete key press to trigger file deletion
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)) && !selectedFilePath.empty()) {
        try {
            std::filesystem::remove(selectedFilePath);
            console.addLog(("Archivo eliminado: " + selectedFilePath).c_str());
            selectedFilePath.clear();
        }
        catch (const std::filesystem::filesystem_error& e) {
            console.addLog(("Error al eliminar archivo: " + std::string(e.what())).c_str());
        }
    }

    for (const auto& entry : std::filesystem::directory_iterator(libraryPath)) {
        std::string filePath = entry.path().string();
        std::string fileName = entry.path().filename().string();

        if (entry.is_directory()) {
            ImGui::PushID(filePath.c_str());

            bool isExpanded = folderExpanded[filePath];
            if (ImGui::TreeNode(fileName.c_str())) {
                folderExpanded[filePath] = true;

                for (const auto& subEntry : std::filesystem::directory_iterator(filePath)) {
                    std::string subFilePath = subEntry.path().string();
                    std::string subFileName = subEntry.path().filename().string();

                    if (subEntry.is_regular_file()) {
                        ImGui::PushID(subFilePath.c_str());
                        if (ImGui::Selectable(subFileName.c_str(), selectedFilePath == subFilePath)) {
                            selectedFilePath = subFilePath;
                        }
                        ImGui::PopID();
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
            std::string extension = entry.path().extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

            ImGui::PushID(filePath.c_str());
            if (ImGui::Selectable(fileName.c_str(), selectedFilePath == filePath)) {
                selectedFilePath = filePath;
            }

            ImGui::PopID();
        }
    }
    ImGui::End();
}