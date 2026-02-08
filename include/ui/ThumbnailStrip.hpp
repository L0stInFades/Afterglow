#pragma once

#include <windows.h>
#include <d2d1_1.h>
#include <wrl/client.h>
#include <vector>
#include <memory>
#include <filesystem>
#include <functional>

namespace UltraImageViewer {
namespace UI {

/**
 * Thumbnail data
 */
struct Thumbnail {
    std::filesystem::path path;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> bitmap;
    uint32_t width;
    uint32_t height;
    bool isLoading;
    bool isFailed;

    Thumbnail() : width(0), height(0), isLoading(false), isFailed(false) {}
};

/**
 * Thumbnail strip with virtualization
 * Shows infinite scrolling gallery of image thumbnails
 */
class ThumbnailStrip {
public:
    using ThumbnailCallback = std::function<void(const std::filesystem::path&)>;

    ThumbnailStrip();
    ~ThumbnailStrip();

    // Initialize
    bool Initialize(HWND hwnd, ID2D1DeviceContext* context);

    // Thumbnails
    void SetThumbnails(const std::vector<std::filesystem::path>& paths);
    void AddThumbnail(const std::filesystem::path& path);
    void ClearThumbnails();

    // Selection
    void SetSelectedIndex(size_t index);
    size_t GetSelectedIndex() const { return selectedIndex_; }
    std::filesystem::path GetSelectedPath() const;

    // Navigation
    void ScrollToIndex(size_t index);
    void Scroll(float delta);
    void Next();
    void Previous();

    // Rendering
    void Render(ID2D1DeviceContext* context);
    void Invalidate();

    // Layout
    void SetPosition(float x, float y, float width, float height);
    void SetThumbnailSize(float size) { thumbnailSize_ = size; InvalidateLayout(); }
    void SetSpacing(float spacing) { spacing_ = spacing; InvalidateLayout(); }

    // Callbacks
    void SetSelectionChangedCallback(ThumbnailCallback callback) {
        selectionChangedCallback_ = std::move(callback);
    }

    // Configuration
    void SetCacheRadius(size_t radius) { cacheRadius_ = radius; }
    void SetBackgroundColor(D2D1_COLOR_F color) { backgroundColor_ = color; }
    void SetSelectedBorderColor(D2D1_COLOR_F color) { selectedBorderColor_ = color; }

    // State
    bool IsVisible() const { return visible_; }
    void SetVisible(bool visible) { visible_ = visible; }

private:
    // Layout calculation
    void InvalidateLayout();
    void CalculateLayout();
    size_t GetVisibleRange() const;
    D2D1_RECT_F GetThumbnailRect(size_t index) const;

    // Thumbnail management
    void LoadThumbnail(size_t index);
    void UnloadThumbnail(size_t index);
    void UpdateVisibleThumbnails();

    // Rendering helpers
    void RenderBackground(ID2D1DeviceContext* context);
    void RenderThumbnails(ID2D1DeviceContext* context);
    void RenderSelection(ID2D1DeviceContext* context, size_t index);
    void RenderLoadingIndicator(ID2D1DeviceContext* context, size_t index);

    // Window and Direct2D
    HWND hwnd_;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> context_;

    // Thumbnails
    std::vector<Thumbnail> thumbnails_;

    // Selection
    size_t selectedIndex_;
    ThumbnailCallback selectionChangedCallback_;

    // Scrolling
    float scrollOffset_;  // Pixels from left edge
    float targetScrollOffset_;
    bool isScrolling_;

    // Layout
    float x_, y_, width_, height_;
    float thumbnailSize_;
    float spacing_;
    size_t firstVisibleIndex_;
    size_t lastVisibleIndex_;

    // Configuration
    size_t cacheRadius_;  // How many thumbnails to load outside visible range
    bool visible_;
    bool layoutDirty_;

    // Colors
    D2D1_COLOR_F backgroundColor_;
    D2D1_COLOR_F selectedBorderColor_;
    D2D1_COLOR_F loadingIndicatorColor_;

    // Rendering resources
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> backgroundBrush_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> selectedBorderBrush_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> loadingBrush_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> failedBrush_;

    // Animation
    float animationSpeed_;
    ULONGLONG lastAnimationTime_;

    // Constants
    static constexpr float DEFAULT_THUMBNAIL_SIZE = 120.0f;
    static constexpr float DEFAULT_SPACING = 8.0f;
    static constexpr size_t DEFAULT_CACHE_RADIUS = 5;
    static constexpr float ANIMATION_SPEED = 0.2f; // Lerp factor
};

} // namespace UI
} // namespace UltraImageViewer
