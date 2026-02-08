#include "pch.h"
#include "App.xaml.h"
#include "MainWindow.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::UltraImageViewer::WinUI::implementation
{
    App::App()
    {
        // Xaml host is required for WinUI 3 unpackaged
        auto windowsXamlManager = Microsoft::UI::Xaml::XamlHosting::WindowsXamlManager::InitializeForCurrentThread();
    }

    void App::OnLaunched(LaunchActivatedEventArgs const&)
    {
        m_window = Window();
        m_window.Content(Make<MainWindow>());
        m_window.Activate();
    }
}
