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
    static bool dragInProgress = false;
    static std::map<std::string, bool> folderExpanded;

    if (!std::filesystem::exists(libraryPath)) {
        ImGui::Text("Library folder not found: %s", libraryPath.c_str());
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

                    if (subEntry.is_regular_file()) {
                        std::string subFilePath = subEntry.path().string();
                        std::string subFileName = subEntry.path().filename().string();
                        std::string extension = subEntry.path().extension().string();
                       
                        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                        ImGui::PushID(subFilePath.c_str());
                        if (ImGui::Selectable(subFileName.c_str(), selectedFilePath == subFilePath)) {
                            selectedFilePath = subFilePath;
                        }
                        if (ImGui::BeginPopupContextItem()) {

                            float initialX = ImGui::GetCursorPosX();
                            ImGui::SetCursorPosX(initialX + 20.0f);

                            if (ImGui::Selectable("Delete")) {
                                try {
                                    std::filesystem::remove(subFilePath);
                                    console.addLog(("Deleted file: " + subFilePath).c_str());
                                    if (selectedFilePath == subFilePath) {
                                        selectedFilePath.clear();
                                    }
                                }
                                catch (const std::filesystem::filesystem_error& e) {
                                    console.addLog(("Error deleting file: " + std::string(e.what())).c_str());
                                }
                            }
                            ImGui::EndPopup();
                        }

                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                            ImGui::SetDragDropPayload("LibraryFile", subFilePath.c_str(), subFilePath.length() + 1);
                            ImGui::Text("Drag: %s", subFileName.c_str());
                            ImGui::EndDragDropSource();
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

            if (ImGui::BeginPopupContextItem()) {

                float initialX = ImGui::GetCursorPosX();
                ImGui::SetCursorPosX(initialX + 20.0f);

                if (ImGui::Selectable("Delete")) {
                    try {
                        std::filesystem::remove(filePath);
                        console.addLog(("Deleted file: " + filePath).c_str());
                        if (selectedFilePath == filePath) {
                            selectedFilePath.clear();
                        }
                    }
                    catch (const std::filesystem::filesystem_error& e) {
                        console.addLog(("Error deleting file: " + std::string(e.what())).c_str());
                    }
                }
                ImGui::EndPopup();
            }

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                ImGui::SetDragDropPayload("LibraryFile", filePath.c_str(), filePath.length() + 1);
                ImGui::Text("Drag: %s", fileName.c_str());
                ImGui::EndDragDropSource();
            }
            ImGui::PopID();
        }
    }
    ImGui::End();
}
