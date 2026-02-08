#pragma once

#include <windows.h>
#include <memory>

namespace UltraImageViewer {

namespace Core {
class Application;
}

namespace UI {

/**
 * Main UI window manager
 * Handles the primary user interface and interactions
 */
class MainWindow {
public:
    explicit MainWindow(Core::Application* app);
    ~MainWindow();

    bool Initialize();
    void Shutdown();

    void OnRender();
    void OnResize(UINT width, UINT height);

private:
    Core::Application* app_;
    bool isInitialized_ = false;
};

} // namespace UI
} // namespace UltraImageViewer
