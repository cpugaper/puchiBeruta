#include "ProjectWindow.h"
#include "ConsoleWindow.h"
#include "Renderer.h"
#include "imgui.h"
#include <iostream>

extern Renderer renderer;
extern ConsoleWindow console;

void ProjectWindow::render() {
    ImGui::Begin("Project", nullptr);

    static std::string assetsPath = "Assets";
    static std::string selectedFilePath;
    static bool dragInProgress = false;

    if (!std::filesystem::exists(assetsPath)) {
        ImGui::Text("Assets folder not found: %s", assetsPath.c_str());
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

    for (const auto& entry : std::filesystem::directory_iterator(assetsPath)) {
        if (entry.is_regular_file()) {
            std::string filePath = entry.path().string();
            std::string fileName = entry.path().filename().string();
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
                ImGui::SetDragDropPayload("AssetFile", filePath.c_str(), filePath.length() + 1);
                ImGui::Text("Drag: %s", fileName.c_str());
                ImGui::EndDragDropSource();
            }
            ImGui::PopID();
        }
    }

    ImGui::End();
}