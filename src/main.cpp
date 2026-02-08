#include <windows.h>
#include <wrl/client.h>
#include <string>
#include <memory>
#include <filesystem>

#include "core/Application.hpp"
#include "core/ImagePipeline.hpp"

using namespace Microsoft::WRL;

// Entry point
int APIENTRY wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nCmdShow)
{
    // Enable per-monitor DPI awareness V2 (Windows 10 1703+)
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // Initialize COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        return 1;
    }

    // Create and run application
    auto app = std::make_unique<UltraImageViewer::Core::Application>();

    int exitCode = 0;
    try {
        if (app->Initialize(hInstance)) {
            // Handle command-line argument (file or directory path)
            if (lpCmdLine && lpCmdLine[0] != L'\0') {
                std::wstring raw = lpCmdLine;

                // Strip surrounding quotes if present
                if (raw.size() >= 2 && raw.front() == L'"' && raw.back() == L'"') {
                    raw = raw.substr(1, raw.size() - 2);
                }
                // Also handle case where quotes are only at front (Windows sometimes)
                while (!raw.empty() && raw.front() == L'"') raw.erase(raw.begin());
                while (!raw.empty() && raw.back() == L'"') raw.pop_back();

                // Trim whitespace
                while (!raw.empty() && raw.back() == L' ') raw.pop_back();

                std::filesystem::path cmdPath(raw);

                OutputDebugStringW((L"[UIV] cmdline raw: " + raw + L"\n").c_str());
                OutputDebugStringW((L"[UIV] path exists: " + std::wstring(std::filesystem::exists(cmdPath) ? L"yes" : L"no") + L"\n").c_str());

                if (std::filesystem::exists(cmdPath)) {
                    if (std::filesystem::is_directory(cmdPath)) {
                        auto images = UltraImageViewer::Core::ImagePipeline::ScanDirectory(cmdPath);
                        OutputDebugStringW((L"[UIV] scan found: " + std::to_wstring(images.size()) + L" images\n").c_str());
                        if (!images.empty()) {
                            app->OpenImages(images);
                        }
                    } else {
                        app->OpenImages({cmdPath});
                    }
                }
            }
            exitCode = app->Run(nCmdShow);
        } else {
            exitCode = 1;
        }
    } catch (const std::exception& e) {
        std::string msg = "Fatal error: ";
        msg += e.what();
        MessageBoxA(nullptr, msg.c_str(), "UltraImageViewer", MB_ICONERROR);
        exitCode = 1;
    }

    app->Shutdown();
    app.reset();

    CoUninitialize();
    return exitCode;
}
