#pragma once
#include "App.xaml.g.h"

namespace winrt::UltraImageViewer::WinUI::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

    private:
        Microsoft::UI::Xaml::Window m_window{ nullptr };
    };
}

namespace winrt::UltraImageViewer::WinUI::factory_implementation
{
    struct App : AppT<App, winrt::UltraImageViewer::WinUI::implementation::App>
    {
    };
}
