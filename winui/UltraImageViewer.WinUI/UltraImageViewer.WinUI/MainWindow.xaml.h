#pragma once
#include "MainWindow.xaml.g.h"
#include <vector>

namespace winrt::UltraImageViewer::WinUI::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        // Event handlers
        void OnOpenClicked(IInspectable const&, RoutedEventArgs const&);
        void OnRecentItemClick(IInspectable const{}, ItemClickEventArgs const& args);
        void OnTabCloseRequested(IInspectable const{}, TabViewTabCloseRequestedEventArgs const& args);

        // Initialization
        void InitializeAnimation();
        void InitializeKeyboardShortcuts();
        void LoadRecentFiles();

        // Helper functions
        fire_and_forget OpenFilesAsync();
        fire_and_forget OpenFileAsync(winrt::Windows::Storage::StorageFile const& file);
        void AddImageTab(winrt::Windows::Storage::StorageFile const& file);
        void SwitchToMainView();
        void SwitchToWelcomeView();
        fire_and_forget SaveRecentAsync();
        fire_and_forget FitImageToViewport(Microsoft::UI::Xaml::Controls::ScrollViewer const& sv, Microsoft::UI::Xaml::Controls::Image const& img);

    private:
        Microsoft::UI::Xaml::DispatcherTimer m_animationTimer{ nullptr };
        int m_animFrame = 0;
        std::vector<std::wstring> m_animFrames = {
            L"[=    ]",
            L"[==   ]",
            L"[===  ]",
            L"[==== ]",
            L"[=====]",
            L"[ ====]",
            L"[  ===]",
            L"[   ==]",
            L"[    =]"
        };
        std::vector<std::wstring> m_recentFiles;
        const size_t MAX_RECENT = 10;
        HWND m_hwnd = nullptr;
        Microsoft::UI::Xaml::Window m_window{ nullptr };
    };
}

namespace winrt::UltraImageViewer::WinUI::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, winrt::UltraImageViewer::WinUI::implementation::MainWindow>
    {
    };
}
