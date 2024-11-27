#include "ProjectWindow.h"
#include "Renderer.h"
#include "imgui.h"
#include <iostream>

extern Renderer renderer;

void ProjectWindow::render() {
    ImGui::Begin("Project", nullptr);
    static std::string assetsPath = "Assets";
    if (!std::filesystem::exists(assetsPath)) {
        ImGui::Text("Assets folder not found: %s", assetsPath.c_str());
        ImGui::End();
        return;
    }
    for (const auto& entry : std::filesystem::directory_iterator(assetsPath)) {
        if (entry.is_regular_file()) {
            std::string filePath = entry.path().string();
            std::string fileName = entry.path().filename().string();
            std::string extension = entry.path().extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            if (ImGui::Button(fileName.c_str())) {
                renderer.HandleDroppedFile(filePath.c_str());
            }
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                ImGui::SetDragDropPayload("AssetFile", filePath.c_str(), filePath.length() + 1);
                ImGui::Text("Arrastra: %s", entry.path().filename().string().c_str());
                ImGui::EndDragDropSource();
            }
        }
    }

    ImGui::End();
}