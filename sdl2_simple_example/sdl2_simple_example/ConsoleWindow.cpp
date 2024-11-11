#include "ConsoleWindow.h"
#include <imgui.h>

ConsoleWindow::ConsoleWindow() {}

ConsoleWindow::~ConsoleWindow() {}

void ConsoleWindow::addLog(const std::string& log) {
    if (logs.size() >= maxLogs) {
        logs.erase(logs.begin()); 
    }
    logs.push_back(log);
}

void ConsoleWindow::clearLogs() {
    logs.clear();
}

void ConsoleWindow::display() {
    ImGui::Begin("Console");

    if (ImGui::Button("Clear")) {
        clearLogs();
    }

    ImGui::Separator();

    for (const auto& log : logs) {
        ImGui::TextUnformatted(log.c_str());
    }

    ImGui::End();
}


