#pragma once

#include <string>
#include <vector>
#include <functional>
#include <windows.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <wrl/client.h>

namespace UltraImageViewer {
namespace UI {

struct Command {
    std::string id;
    std::string title;
    std::string description;
    std::string category;
    std::vector<std::string> keywords;
    std::function<void()> action;

    Command() = default;
    Command(std::string id, std::string title, std::string description,
            std::string category, std::function<void()> action)
        : id(std::move(id))
        , title(std::move(title))
        , description(std::move(description))
        , category(std::move(category))
        , action(std::move(action))
    {
    }
};

struct CommandMatch {
    const Command* command;
    int score;

    CommandMatch() : command(nullptr), score(0) {}
    CommandMatch(const Command* cmd, int s) : command(cmd), score(s) {}
};

/**
 * VSCode-style command palette with fuzzy search
 * Activated via Ctrl+Shift+P
 */
class CommandPalette {
public:
    CommandPalette();
    ~CommandPalette();

    // Initialize with window handle
    bool Initialize(HWND hwnd);

    // Show/hide palette
    void Show();
    void Hide();
    bool IsVisible() const { return isVisible_; }

    // Commands
    void RegisterCommand(const Command& command);
    void UnregisterCommand(const std::string& id);
    void ClearCommands();

    // Input handling
    void OnChar(WCHAR ch);
    void OnKeyDown(UINT key);
    void OnKeyUp(UINT key);

    // Rendering (called from main render loop)
    void Render(ID2D1DeviceContext* context);

    // Set theme colors
    void SetColors(
        D2D1_COLOR_F background,
        D2D1_COLOR_F text,
        D2D1_COLOR_F selected,
        D2D1_COLOR_F border);

private:
    // Fuzzy search algorithm (substring matching with ranking)
    std::vector<CommandMatch> FuzzySearch(const std::wstring& query) const;

    // Calculate match score for fuzzy search
    int CalculateMatchScore(const std::wstring& query, const std::wstring& text) const;

    // Update selected index
    void MoveSelection(int delta);
    void ExecuteSelected();

    // UI state
    HWND hwnd_;
    bool isVisible_;
    std::wstring query_;
    size_t selectedIndex_;
    std::vector<CommandMatch> filteredCommands_;

    // All registered commands
    std::vector<Command> commands_;

    // Theme colors
    D2D1_COLOR_F backgroundColor_;
    D2D1_COLOR_F textColor_;
    D2D1_COLOR_F selectedColor_;
    D2D1_COLOR_F borderColor_;

    // Layout
    float x_, y_, width_, height_;
    static constexpr float ITEM_HEIGHT = 32.0f;
    static constexpr float MAX_VISIBLE_ITEMS = 10.0f;
    static constexpr size_t MAX_QUERY_LENGTH = 256;

    // Rendering resources (created on first show)
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> backgroundBrush_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> textBrush_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> selectedBrush_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> borderBrush_;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat_;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> categoryFormat_;

    // Helper methods
    D2D1_RECT_F GetPaletteRect() const;
    D2D1_RECT_F GetQueryRect() const;
    D2D1_RECT_F GetListRect() const;
    D2D1_RECT_F GetItemRect(size_t index) const;
    size_t GetScrollOffset() const;
};

} // namespace UI
} // namespace UltraImageViewer
