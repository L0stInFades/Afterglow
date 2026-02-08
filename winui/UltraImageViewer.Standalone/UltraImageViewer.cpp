// UltraImageViewer - Standalone Win32 Version
// Compile: cl UltraImageViewer.cpp /EHsc /link user32.lib gdiplus.lib shlwapi.lib shell32.lib ole32.lib

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <comdef.h>
#include <string>
#include <vector>
#include <fstream>
#include <shlobj.h>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

using namespace Gdiplus;
using namespace std;

// Global variables
HWND g_hWnd = NULL;
HINSTANCE g_hInst = NULL;
vector<wstring> g_recentFiles;
const int MAX_RECENT = 10;
wstring g_currentFile;
Image* g_pImage = NULL;
float g_zoom = 1.0f;
int g_panX = 0, g_panY = 0;
bool g_dragging = false;
POINT g_dragStart;
ULONG_PTR g_gdiplusToken = 0;

// Recent file path
wstring GetRecentFilePath() {
    wchar_t path[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path) == S_OK) {
        wcscat_s(path, L"\\UltraImageViewer_Recent.txt");
        return wstring(path);
    }
    return L"";
}

void LoadRecentFiles() {
    wstring path = GetRecentFilePath();
    FILE* f = NULL;
    if (_wfopen_s(&f, path.c_str(), L"r, ccs=UTF-8") == 0 && f) {
        wchar_t line[MAX_PATH * 2];
        g_recentFiles.clear();
        while (fgetws(line, _countof(line), f)) {
            // Remove newline
            wchar_t* newline = wcschr(line, L'\n');
            if (newline) *newline = 0;
            newline = wcschr(line, L'\r');
            if (newline) *newline = 0;

            if (wcslen(line) > 0 && PathFileExists(line)) {
                g_recentFiles.push_back(line);
            }
        }
        fclose(f);
    }
}

void SaveRecentFiles() {
    wstring path = GetRecentFilePath();
    FILE* f = NULL;
    if (_wfopen_s(&f, path.c_str(), L"w, ccs=UTF-8") == 0 && f) {
        for (const auto& file : g_recentFiles) {
            fwprintf(f, L"%s\n", file.c_str());
        }
        fclose(f);
    }
}

void AddToRecent(const wstring& file) {
    // Remove if exists
    auto it = find(g_recentFiles.begin(), g_recentFiles.end(), file);
    if (it != g_recentFiles.end()) {
        g_recentFiles.erase(it);
    }

    // Add to front
    g_recentFiles.insert(g_recentFiles.begin(), file);

    // Limit to MAX_RECENT
    while (g_recentFiles.size() > MAX_RECENT) {
        g_recentFiles.pop_back();
    }

    SaveRecentFiles();
}

void FitImageToWindow() {
    if (!g_pImage) return;

    RECT rc;
    GetClientRect(g_hWnd, &rc);
    int imgWidth = g_pImage->GetWidth();
    int imgHeight = g_pImage->GetHeight();

    float scaleX = (float)(rc.right - rc.left) / imgWidth;
    float scaleY = (float)(rc.bottom - rc.top) / imgHeight;
    g_zoom = min(scaleX, scaleY);
    if (g_zoom > 1.0f) g_zoom = 1.0f;

    g_panX = 0;
    g_panY = 0;
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void LoadImage(const wchar_t* path) {
    if (g_pImage) {
        delete g_pImage;
        g_pImage = NULL;
    }

    g_pImage = Image::FromFile(path);
    if (g_pImage && g_pImage->GetLastStatus() == Ok) {
        g_currentFile = path;
        g_panX = 0;
        g_panY = 0;
        AddToRecent(path);

        // Auto fit to window
        FitImageToWindow();
    }
}

void OpenFile() {
    OPENFILENAME ofn = {0};
    wchar_t szFile[MAX_PATH] = L"";

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Image Files\0*.jpg;*.jpeg;*.png;*.gif;*.bmp;*.webp;*.tiff\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ALLOWMULTISELECT;

    if (GetOpenFileName(&ofn)) {
        // Handle file selection
        if (ofn.nFileOffset == 0) {
            // Only one file selected (path in szFile)
            LoadImage(szFile);
        } else {
            // Multiple files selected
            wchar_t path[MAX_PATH];
            wcscpy_s(path, szFile);

            wchar_t* p = szFile + ofn.nFileOffset;
            while (*p) {
                wchar_t fullPath[MAX_PATH];
                wcscpy_s(fullPath, path);
                PathAppend(fullPath, p);

                // Load first image
                if (g_currentFile.empty()) {
                    LoadImage(fullPath);
                }

                p += wcslen(p) + 1;
            }
        }
    }
}

void DrawWelcomePage(HDC hdc, RECT& rc) {
    // Background
    HBRUSH hBrush = CreateSolidBrush(RGB(30, 30, 30));
    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);

    // Title
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);

    LOGFONT lf = {0};
    lf.lfHeight = 48;
    lf.lfWeight = FW_BOLD;
    wcscpy_s(lf.lfFaceName, L"Arial");

    HFONT hFont = CreateFontIndirect(&lf);
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    RECT rcText = rc;
    DrawText(hdc, L"UltraImageViewer", -1, &rcText, DT_CENTER | DT_TOP | DT_SINGLELINE);

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);

    // Instructions
    lf.lfHeight = 20;
    lf.lfWeight = FW_NORMAL;
    hFont = CreateFontIndirect(&lf);
    hOldFont = (HFONT)SelectObject(hdc, hFont);

    rcText.top = 100;
    DrawText(hdc, L"Press Ctrl+O to open an image", -1, &rcText, DT_CENTER | DT_TOP | DT_SINGLELINE);

    // Recent files
    if (!g_recentFiles.empty()) {
        rcText.top = 200;
        DrawText(hdc, L"Recent Files:", -1, &rcText, DT_CENTER | DT_TOP | DT_SINGLELINE);

        rcText.top = 240;
        rcText.left = 100;
        rcText.right = rc.right - 100;
        for (size_t i = 0; i < g_recentFiles.size() && i < 10; i++) {
            wstring filename = g_recentFiles[i];
            size_t pos = filename.find_last_of(L"\\/");
            if (pos != wstring::npos) {
                filename = filename.substr(pos + 1);
            }
            DrawText(hdc, filename.c_str(), -1, &rcText, DT_LEFT | DT_TOP | DT_SINGLELINE);
            rcText.top += 30;
        }
    }

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            // Initialize GDI+
            GdiplusStartupInput gdiplusStartupInput;
            GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
            LoadRecentFiles();
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rc;
            GetClientRect(hWnd, &rc);

            if (!g_pImage) {
                DrawWelcomePage(hdc, rc);
            } else {
                // Fill background
                HBRUSH hBrush = CreateSolidBrush(RGB(20, 20, 20));
                FillRect(hdc, &rc, hBrush);
                DeleteObject(hBrush);

                // Create graphics
                Graphics graphics(hdc);
                graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
                graphics.SetSmoothingMode(SmoothingModeAntiAlias);

                // Calculate image position
                int imgWidth = (int)(g_pImage->GetWidth() * g_zoom);
                int imgHeight = (int)(g_pImage->GetHeight() * g_zoom);
                int x = (rc.right - rc.left - imgWidth) / 2 + g_panX;
                int y = (rc.bottom - rc.top - imgHeight) / 2 + g_panY;

                // Draw image
                graphics.DrawImage(g_pImage, x, y, imgWidth, imgHeight);
            }

            EndPaint(hWnd, &ps);
            break;
        }

        case WM_KEYDOWN: {
            if (GetKeyState(VK_CONTROL) < 0) {
                if (wParam == 'O') {
                    OpenFile();
                }
            }
            if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {
                g_zoom *= 1.2f;
                InvalidateRect(hWnd, NULL, TRUE);
            }
            if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
                g_zoom /= 1.2f;
                InvalidateRect(hWnd, NULL, TRUE);
            }
            if (wParam == '0') {
                FitImageToWindow();
            }
            break;
        }

        case WM_LBUTTONDOWN: {
            if (g_pImage) {
                g_dragging = true;
                g_dragStart.x = LOWORD(lParam);
                g_dragStart.y = HIWORD(lParam);
                SetCapture(hWnd);
            }
            break;
        }

        case WM_LBUTTONUP: {
            g_dragging = false;
            ReleaseCapture();
            break;
        }

        case WM_MOUSEMOVE: {
            if (g_dragging && g_pImage) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                g_panX += x - g_dragStart.x;
                g_panY += y - g_dragStart.y;
                g_dragStart.x = x;
                g_dragStart.y = y;
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;
        }

        case WM_MOUSEWHEEL: {
            if (g_pImage) {
                int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                if (delta > 0) {
                    g_zoom *= 1.1f;
                } else {
                    g_zoom /= 1.1f;
                }
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;
        }

        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            UINT nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

            if (nFiles > 0) {
                wchar_t szFile[MAX_PATH];
                DragQueryFile(hDrop, 0, szFile, MAX_PATH);
                LoadImage(szFile);
            }
            DragFinish(hDrop);
            break;
        }

        case WM_SIZE: {
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        case WM_DESTROY: {
            if (g_pImage) {
                delete g_pImage;
            }
            if (g_gdiplusToken) {
                GdiplusShutdown(g_gdiplusToken);
            }
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_hInst = hInstance;

    // Register window class
    WNDCLASSEX wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"UltraImageViewerClass";

    RegisterClassEx(&wcex);

    // Create window
    g_hWnd = CreateWindowEx(0, L"UltraImageViewerClass", L"UltraImageViewer",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          1200, 800,
                          NULL, NULL, hInstance, NULL);

    if (!g_hWnd) return 1;

    // Enable drag and drop
    DragAcceptFiles(g_hWnd, TRUE);

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
