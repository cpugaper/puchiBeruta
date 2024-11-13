#ifndef CONSOLE_WINDOW_H
#define CONSOLE_WINDOW_H

#include <string>
#include <vector>
#include <imgui.h>

class ConsoleWindow {
public:
    ConsoleWindow();
    ~ConsoleWindow();

    void addLog(const std::string& log);
    void clearLogs();
    void displayConsole();

    bool autoScroll;
    bool scrollToBottom;

    std::vector<std::string> logs;
    static const size_t maxLogs = 100;
};

extern ConsoleWindow console;

#endif // CONSOLE_WINDOW_H
