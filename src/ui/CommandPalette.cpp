#include "ui/CommandPalette.hpp"
#include <algorithm>
#include <cctype>
#include <cwctype>
#include <d2d1.h>
#include <dwrite.h>

namespace UltraImageViewer {
namespace UI {

CommandPalette::CommandPalette()
    : hwnd_(nullptr)
    , isVisible_(false)
    , selectedIndex_(0)
    , backgroundColor_{0.25f, 0.25f, 0.25f, 0.95f}
    , textColor_{0.9f, 0.9f, 0.9f, 1.0f}
    , selectedColor_{0.0f, 0.5f, 1.0f, 0.3f}
    , borderColor_{0.5f, 0.5f, 0.5f, 1.0f}
    , x_(100.0f)
    , y_(100.0f)
    , width_(600.0f)
    , height_(400.0f)
{
}

CommandPalette::~CommandPalette() = default;

bool CommandPalette::Initialize(HWND hwnd)
{
    hwnd_ = hwnd;
    return true;
}

void CommandPalette::Show()
{
    isVisible_ = true;
    query_.clear();
    selectedIndex_ = 0;
    filteredCommands_.clear();

    // Show all commands initially
    for (const auto& cmd : commands_) {
        filteredCommands_.emplace_back(&cmd, 0);
    }

    // Sort by category and title
    std::sort(filteredCommands_.begin(), filteredCommands_.end(),
        [](const CommandMatch& a, const CommandMatch& b) {
            if (a.command->category != b.command->category) {
                return a.command->category < b.command->category;
            }
            return a.command->title < b.command->title;
        });

    // Set focus to capture input
    SetFocus(hwnd_);
}

void CommandPalette::Hide()
{
    isVisible_ = false;
    query_.clear();
    filteredCommands_.clear();
}

void CommandPalette::RegisterCommand(const Command& command)
{
    commands_.push_back(command);
}

void CommandPalette::UnregisterCommand(const std::string& id)
{
    commands_.erase(
        std::remove_if(commands_.begin(), commands_.end(),
            [&id](const Command& cmd) { return cmd.id == id; }),
        commands_.end());
}

void CommandPalette::ClearCommands()
{
    commands_.clear();
}

void CommandPalette::OnChar(WCHAR ch)
{
    if (!isVisible_) {
        return;
    }

    // Handle printable characters
    if (ch >= 32 && ch < 127 && query_.length() < MAX_QUERY_LENGTH) {
        query_ += towlower(ch);
        selectedIndex_ = 0;

        // Update filtered commands
        filteredCommands_ = FuzzySearch(query_);
    }
}

void CommandPalette::OnKeyDown(UINT key)
{
    if (!isVisible_) {
        return;
    }

    switch (key) {
        case VK_ESCAPE:
            Hide();
            break;

        case VK_RETURN:
            ExecuteSelected();
            Hide();
            break;

        case VK_UP:
            MoveSelection(-1);
            break;

        case VK_DOWN:
            MoveSelection(1);
            break;

        case VK_PRIOR: // Page Up
            selectedIndex_ = (selectedIndex_ >= MAX_VISIBLE_ITEMS)
                ? selectedIndex_ - static_cast<size_t>(MAX_VISIBLE_ITEMS)
                : 0;
            break;

        case VK_NEXT: // Page Down
            selectedIndex_ = std::min(
                selectedIndex_ + static_cast<size_t>(MAX_VISIBLE_ITEMS),
                filteredCommands_.empty() ? 0 : filteredCommands_.size() - 1);
            break;

        case VK_HOME:
            selectedIndex_ = 0;
            break;

        case VK_END:
            selectedIndex_ = filteredCommands_.empty()
                ? 0
                : filteredCommands_.size() - 1;
            break;

        case VK_BACK:
            if (!query_.empty()) {
                query_.pop_back();
                selectedIndex_ = 0;
                filteredCommands_ = FuzzySearch(query_);
            }
            break;
    }
}

void CommandPalette::OnKeyUp(UINT key)
{
    // Not used currently
}

void CommandPalette::Render(ID2D1DeviceContext* context)
{
    if (!isVisible_) {
        return;
    }

    // TODO: Create brushes and text formats on first render
    // For now, this is a placeholder
    //
    // The actual rendering would:
    // 1. Draw background rectangle
    // 2. Draw border
    // 3. Draw query text
    // 4. Draw command list with selection highlight
    // 5. Draw category labels
    // 6. Draw scroll indicator if needed
}

void CommandPalette::SetColors(
    D2D1_COLOR_F background,
    D2D1_COLOR_F text,
    D2D1_COLOR_F selected,
    D2D1_COLOR_F border)
{
    backgroundColor_ = background;
    textColor_ = text;
    selectedColor_ = selected;
    borderColor_ = border;

    // Recreate brushes
    backgroundBrush_.Reset();
    textBrush_.Reset();
    selectedBrush_.Reset();
    borderBrush_.Reset();
}

std::vector<CommandMatch> CommandPalette::FuzzySearch(const std::wstring& query) const
{
    std::vector<CommandMatch> matches;

    if (query.empty()) {
        // Return all commands
        for (const auto& cmd : commands_) {
            matches.emplace_back(&cmd, 0);
        }

        // Sort by category and title
        std::sort(matches.begin(), matches.end(),
            [](const CommandMatch& a, const CommandMatch& b) {
                if (a.command->category != b.command->category) {
                    return a.command->category < b.command->category;
                }
                return a.command->title < b.command->title;
            });

        return matches;
    }

    // Search for matching commands
    for (const auto& cmd : commands_) {
        // Convert title to lowercase for comparison
        std::wstring titleLower;
        titleLower.reserve(cmd.title.length());
        for (WCHAR ch : cmd.title) {
            titleLower += towlower(ch);
        }

        // Calculate match score
        int score = CalculateMatchScore(query, titleLower);

        if (score > 0) {
            matches.emplace_back(&cmd, score);
        }
    }

    // Sort by score (descending)
    std::sort(matches.begin(), matches.end(),
        [](const CommandMatch& a, const CommandMatch& b) {
            return a.score > b.score;
        });

    return matches;
}

int CommandPalette::CalculateMatchScore(const std::wstring& query, const std::wstring& text) const
{
    // Simple substring matching with scoring
    // Higher score for:
    // - Matches at start of string
    // - Matches at word boundaries
    // - Consecutive character matches

    size_t queryPos = 0;
    int score = 0;
    bool inWord = false;
    int consecutiveMatches = 0;

    for (size_t i = 0; i < text.length() && queryPos < query.length(); ++i) {
        if (text[i] == query[queryPos]) {
            // Found a match

            // Bonus for start of string
            if (i == 0) {
                score += 100;
            }

            // Bonus for word boundary
            if (i == 0 || !iswalnum(text[i - 1])) {
                score += 50;
            }

            // Bonus for consecutive matches
            consecutiveMatches++;
            score += consecutiveMatches * 10;

            queryPos++;
            inWord = true;
        } else {
            consecutiveMatches = 0;
            inWord = false;
        }
    }

    // Only count as match if entire query was found
    if (queryPos < query.length()) {
        return 0;
    }

    // Penalty for longer text (prefer shorter matches)
    score -= static_cast<int>(text.length());

    return std::max(0, score);
}

void CommandPalette::MoveSelection(int delta)
{
    if (filteredCommands_.empty()) {
        return;
    }

    int newIndex = static_cast<int>(selectedIndex_) + delta;

    // Clamp to valid range
    if (newIndex < 0) {
        newIndex = 0;
    } else if (newIndex >= static_cast<int>(filteredCommands_.size())) {
        newIndex = static_cast<int>(filteredCommands_.size()) - 1;
    }

    selectedIndex_ = static_cast<size_t>(newIndex);
}

void CommandPalette::ExecuteSelected()
{
    if (selectedIndex_ >= filteredCommands_.size()) {
        return;
    }

    const Command* cmd = filteredCommands_[selectedIndex_].command;
    if (cmd && cmd->action) {
        cmd->action();
    }
}

D2D1_RECT_F CommandPalette::GetPaletteRect() const
{
    return D2D1::RectF(x_, y_, x_ + width_, y_ + height_);
}

D2D1_RECT_F CommandPalette::GetQueryRect() const
{
    D2D1_RECT_F palette = GetPaletteRect();
    return D2D1::RectF(
        palette.left + 4.0f,
        palette.top + 4.0f,
        palette.right - 4.0f,
        palette.top + 40.0f
    );
}

D2D1_RECT_F CommandPalette::GetListRect() const
{
    D2D1_RECT_F palette = GetPaletteRect();
    return D2D1::RectF(
        palette.left + 4.0f,
        palette.top + 44.0f,
        palette.right - 4.0f,
        palette.bottom - 4.0f
    );
}

D2D1_RECT_F CommandPalette::GetItemRect(size_t index) const
{
    D2D1_RECT_F list = GetListRect();
    size_t scrollOffset = GetScrollOffset();

    float top = list.top + (index - scrollOffset) * ITEM_HEIGHT;
    float bottom = top + ITEM_HEIGHT;

    return D2D1::RectF(list.left, top, list.right, bottom);
}

size_t CommandPalette::GetScrollOffset() const
{
    // Keep selected item visible
    if (selectedIndex_ < static_cast<size_t>(MAX_VISIBLE_ITEMS) / 2) {
        return 0;
    }

    size_t maxOffset = filteredCommands_.empty()
        ? 0
        : (filteredCommands_.size() > static_cast<size_t>(MAX_VISIBLE_ITEMS))
            ? filteredCommands_.size() - static_cast<size_t>(MAX_VISIBLE_ITEMS)
            : 0;

    size_t offset = selectedIndex_ - static_cast<size_t>(MAX_VISIBLE_ITEMS) / 2;
    return std::min(offset, maxOffset);
}

} // namespace UI
} // namespace UltraImageViewer
