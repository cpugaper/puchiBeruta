#include "ProjectWindow.h"
#include "ConsoleWindow.h"
#include "Renderer.h"
#include "imgui.h"
#include <iostream>
#include <map>

extern Renderer renderer;
extern ConsoleWindow console;

namespace {
    void handleFileDeletion(const std::string& filePath, std::string& selectedFilePath) {
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

        ImGui::PushID(filePath.c_str());

        if (ImGui::Selectable(fileName.c_str(), selectedFilePath == filePath)) {
            selectedFilePath = filePath;
        }

        renderFileContextMenu(filePath, selectedFilePath);
        renderFileDragDrop(filePath, fileName);

        ImGui::PopID();
    }
}

void ProjectWindow::render() {
    ImGui::Begin("Project", nullptr);

    static std::string libraryPath = "Library";
    static std::string selectedFilePath;
    static std::map<std::string, bool> folderExpanded;

    if (!std::filesystem::exists(libraryPath)) {
        ImGui::Text("Library folder not found: %s", libraryPath.c_str());
        ImGui::End();
        return;
    }

    // Handle Delete key press
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)) && !selectedFilePath.empty()) {
        handleFileDeletion(selectedFilePath, selectedFilePath);
    }

    // Render directory structure
    for (const auto& entry : std::filesystem::directory_iterator(libraryPath)) {
        if (entry.is_directory()) {
            std::string filePath = entry.path().string();
            std::string fileName = entry.path().filename().string();

            ImGui::PushID(filePath.c_str());

            if (ImGui::TreeNode(fileName.c_str())) {
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