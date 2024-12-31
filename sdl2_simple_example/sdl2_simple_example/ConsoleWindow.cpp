#include "ConsoleWindow.h"
#include <imgui.h>
#include <iostream>
#include <fstream>

ConsoleWindow console;

ConsoleWindow::ConsoleWindow() : autoScroll(true) {}

ConsoleWindow::~ConsoleWindow() {}

void ConsoleWindow::addLog(const std::string& log) {
    if (std::find(logs.begin(), logs.end(), log) == logs.end()) {
        if (logs.size() >= maxLogs) {
            logs.erase(logs.begin());
        }
        logs.push_back(log);

        if (autoScroll) {
            scrollToBottom = true;
        }
    }
}

void ConsoleWindow::clearLogs() {
    logs.clear();
    autoScroll = true;
}

void ConsoleWindow::displayConsole() {
    ImGui::Begin("Console");
    ImGui::BeginChild("LogChild", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    for (const std::string& log : logs) {
        ImGui::Text("%s", log.c_str());
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        autoScroll = true;
    }
    else if (ImGui::GetScrollY() < ImGui::GetScrollMaxY()) {
        autoScroll = false;
    }

    if (autoScroll && scrollToBottom) {
        ImGui::SetScrollHereY(1.0f);
        scrollToBottom = false;
    }

    ImGui::EndChild();
    ImGui::End();
}