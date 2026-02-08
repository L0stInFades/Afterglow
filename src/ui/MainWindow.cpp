#include "ui/MainWindow.hpp"
#include "core/Application.hpp"

namespace UltraImageViewer {
namespace UI {

MainWindow::MainWindow(Core::Application* app)
    : app_(app)
{
}

MainWindow::~MainWindow()
{
    Shutdown();
}

bool MainWindow::Initialize()
{
    isInitialized_ = true;
    return true;
}

void MainWindow::Shutdown()
{
    isInitialized_ = false;
}

void MainWindow::OnRender()
{
    // TODO: Implement UI rendering
}

void MainWindow::OnResize(UINT width, UINT height)
{
    // TODO: Handle resize
}

} // namespace UI
} // namespace UltraImageViewer
