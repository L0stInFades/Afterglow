#include "ui/ThumbnailStrip.hpp"
#include "core/ImageDecoder.hpp"
#include <algorithm>
#include <stdexcept>
#include <cmath>

namespace UltraImageViewer {
namespace UI {

ThumbnailStrip::ThumbnailStrip()
    : hwnd_(nullptr)
    , selectedIndex_(0)
    , scrollOffset_(0.0f)
    , targetScrollOffset_(0.0f)
    , isScrolling_(false)
    , x_(0.0f)
    , y_(0.0f)
    , width_(0.0f)
    , height_(0.0f)
    , thumbnailSize_(DEFAULT_THUMBNAIL_SIZE)
    , spacing_(DEFAULT_SPACING)
    , firstVisibleIndex_(0)
    , lastVisibleIndex_(0)
    , cacheRadius_(DEFAULT_CACHE_RADIUS)
    , visible_(false)
    , layoutDirty_(true)
    , backgroundColor_{0.15f, 0.15f, 0.15f, 1.0f}
    , selectedBorderColor_{0.0f, 0.5f, 1.0f, 1.0f}
    , loadingIndicatorColor_{0.5f, 0.5f, 0.5f, 1.0f}
    , animationSpeed_(ANIMATION_SPEED)
    , lastAnimationTime_(0)
{
}

ThumbnailStrip::~ThumbnailStrip()
{
    ClearThumbnails();
}

bool ThumbnailStrip::Initialize(HWND hwnd, ID2D1DeviceContext* context)
{
    hwnd_ = hwnd;
    context_ = context;

    // Create brushes
    HRESULT hr = context_->CreateSolidColorBrush(backgroundColor_, &backgroundBrush_);
    if (FAILED(hr)) return false;

    hr = context_->CreateSolidColorBrush(selectedBorderColor_, &selectedBorderBrush_);
    if (FAILED(hr)) return false;

    hr = context_->CreateSolidColorBrush(loadingIndicatorColor_, &loadingBrush_);
    if (FAILED(hr)) return false;

    D2D1_COLOR_F failedColor = {0.8f, 0.2f, 0.2f, 1.0f};
    hr = context_->CreateSolidColorBrush(failedColor, &failedBrush_);
    if (FAILED(hr)) return false;

    return true;
}

void ThumbnailStrip::SetThumbnails(const std::vector<std::filesystem::path>& paths)
{
    ClearThumbnails();

    thumbnails_.reserve(paths.size());
    for (const auto& path : paths) {
        Thumbnail thumb;
        thumb.path = path;
        thumbnails_.push_back(std::move(thumb));
    }

    InvalidateLayout();
    UpdateVisibleThumbnails();
}

void ThumbnailStrip::AddThumbnail(const std::filesystem::path& path)
{
    Thumbnail thumb;
    thumb.path = path;
    thumbnails_.push_back(std::move(thumb));

    InvalidateLayout();
    UpdateVisibleThumbnails();
}

void ThumbnailStrip::ClearThumbnails()
{
    thumbnails_.clear();
    selectedIndex_ = 0;
    scrollOffset_ = 0.0f;
    targetScrollOffset_ = 0.0f;
    InvalidateLayout();
}

void ThumbnailStrip::SetSelectedIndex(size_t index)
{
    if (index >= thumbnails_.size()) {
        return;
    }

    if (selectedIndex_ != index) {
        selectedIndex_ = index;

        // Scroll to make selected thumbnail visible
        ScrollToIndex(index);

        // Notify callback
        if (selectionChangedCallback_) {
            selectionChangedCallback_(thumbnails_[index].path);
        }
    }
}

std::filesystem::path ThumbnailStrip::GetSelectedPath() const
{
    if (selectedIndex_ < thumbnails_.size()) {
        return thumbnails_[selectedIndex_].path;
    }
    return std::filesystem::path();
}

void ThumbnailStrip::ScrollToIndex(size_t index)
{
    if (index >= thumbnails_.size()) {
        return;
    }

    // Calculate target offset to center the thumbnail
    float thumbnailTotalSize = thumbnailSize_ + spacing_;
    float targetX = index * thumbnailTotalSize;
    targetScrollOffset_ = targetX - (width_ / 2.0f) + (thumbnailSize_ / 2.0f);

    // Clamp scroll offset
    float maxOffset = std::max(0.0f, (thumbnails_.size() * thumbnailTotalSize) - width_);
    targetScrollOffset_ = std::max(0.0f, std::min(targetScrollOffset_, maxOffset));

    isScrolling_ = true;
}

void ThumbnailStrip::Scroll(float delta)
{
    targetScrollOffset_ += delta;

    // Clamp scroll offset
    float thumbnailTotalSize = thumbnailSize_ + spacing_;
    float maxOffset = std::max(0.0f, (thumbnails_.size() * thumbnailTotalSize) - width_);
    targetScrollOffset_ = std::max(0.0f, std::min(targetScrollOffset_, maxOffset));

    isScrolling_ = true;
}

void ThumbnailStrip::Next()
{
    if (thumbnails_.empty()) {
        return;
    }

    size_t nextIndex = std::min(selectedIndex_ + 1, thumbnails_.size() - 1);
    SetSelectedIndex(nextIndex);
}

void ThumbnailStrip::Previous()
{
    if (thumbnails_.empty()) {
        return;
    }

    size_t prevIndex = selectedIndex_ > 0 ? selectedIndex_ - 1 : 0;
    SetSelectedIndex(prevIndex);
}

void ThumbnailStrip::Render(ID2D1DeviceContext* context)
{
    if (!visible_ || thumbnails_.empty()) {
        return;
    }

    // Update animation
    ULONGLONG currentTime = GetTickCount64();
    if (isScrolling_ && lastAnimationTime_ > 0) {
        float deltaTime = static_cast<float>(currentTime - lastAnimationTime_) / 1000.0f;
        float lerpFactor = std::min(1.0f, animationSpeed_ * deltaTime * 60.0f);

        float diff = targetScrollOffset_ - scrollOffset_;
        if (std::abs(diff) < 0.5f) {
            scrollOffset_ = targetScrollOffset_;
            isScrolling_ = false;
        } else {
            scrollOffset_ += diff * lerpFactor;
        }
    }
    lastAnimationTime_ = currentTime;

    // Calculate layout if dirty
    if (layoutDirty_) {
        CalculateLayout();
    }

    // Render components
    RenderBackground(context);
    RenderThumbnails(context);

    // Render selection border
    if (selectedIndex_ < thumbnails_.size()) {
        RenderSelection(context, selectedIndex_);
    }
}

void ThumbnailStrip::Invalidate()
{
    layoutDirty_ = true;
}

void ThumbnailStrip::SetPosition(float x, float y, float width, float height)
{
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
    InvalidateLayout();
}

void ThumbnailStrip::InvalidateLayout()
{
    layoutDirty_ = true;
}

void ThumbnailStrip::CalculateLayout()
{
    if (thumbnails_.empty()) {
        firstVisibleIndex_ = 0;
        lastVisibleIndex_ = 0;
        return;
    }

    // Calculate visible range
    float thumbnailTotalSize = thumbnailSize_ + spacing_;

    size_t first = static_cast<size_t>(std::max(0.0f, scrollOffset_ / thumbnailTotalSize));
    size_t last = static_cast<size_t>(std::ceil((scrollOffset_ + width_) / thumbnailTotalSize));

    firstVisibleIndex_ = std::max(0ULL, static_cast<ULONGLONG>(first) - cacheRadius_);
    lastVisibleIndex_ = std::min(thumbnails_.size(), last + cacheRadius_);

    layoutDirty_ = false;
}

size_t ThumbnailStrip::GetVisibleRange() const
{
    return lastVisibleIndex_ - firstVisibleIndex_;
}

D2D1_RECT_F ThumbnailStrip::GetThumbnailRect(size_t index) const
{
    float thumbnailTotalSize = thumbnailSize_ + spacing_;
    float x = x_ + (index * thumbnailTotalSize) - scrollOffset_;
    float y = y_ + (height_ - thumbnailSize_) / 2.0f;

    // Calculate aspect ratio fit
    float aspect = 1.0f;  // Default to square
    if (index < thumbnails_.size() && thumbnails_[index].width > 0) {
        aspect = static_cast<float>(thumbnails_[index].width) /
                 static_cast<float>(thumbnails_[index].height);
    }

    float renderWidth, renderHeight;
    if (aspect > 1.0f) {
        renderWidth = thumbnailSize_;
        renderHeight = thumbnailSize_ / aspect;
    } else {
        renderWidth = thumbnailSize_ * aspect;
        renderHeight = thumbnailSize_;
    }

    // Center in slot
    x += (thumbnailSize_ - renderWidth) / 2.0f;
    float adjustedY = y + (thumbnailSize_ - renderHeight) / 2.0f;

    return D2D1::RectF(x, adjustedY, x + renderWidth, adjustedY + renderHeight);
}

void ThumbnailStrip::LoadThumbnail(size_t index)
{
    if (index >= thumbnails_.size()) {
        return;
    }

    auto& thumb = thumbnails_[index];
    if (thumb.bitmap || thumb.isLoading || thumb.isFailed) {
        return;
    }

    thumb.isLoading = true;

    // TODO: Load thumbnail asynchronously
    // For now, mark as failed
    thumb.isLoading = false;
    thumb.isFailed = true;
}

void ThumbnailStrip::UnloadThumbnail(size_t index)
{
    if (index >= thumbnails_.size()) {
        return;
    }

    auto& thumb = thumbnails_[index];
    thumb.bitmap.Reset();
}

void ThumbnailStrip::UpdateVisibleThumbnails()
{
    for (size_t i = firstVisibleIndex_; i < lastVisibleIndex_; ++i) {
        LoadThumbnail(i);
    }

    // Unload thumbnails outside cache range
    for (size_t i = 0; i < firstVisibleIndex_; ++i) {
        UnloadThumbnail(i);
    }
    for (size_t i = lastVisibleIndex_; i < thumbnails_.size(); ++i) {
        UnloadThumbnail(i);
    }
}

void ThumbnailStrip::RenderBackground(ID2D1DeviceContext* context)
{
    D2D1_RECT_F rect = D2D1::RectF(x_, y_, x_ + width_, y_ + height_);
    context->FillRectangle(rect, backgroundBrush_.Get());
}

void ThumbnailStrip::RenderThumbnails(ID2D1DeviceContext* context)
{
    for (size_t i = firstVisibleIndex_; i < lastVisibleIndex_; ++i) {
        if (i >= thumbnails_.size()) {
            break;
        }

        const auto& thumb = thumbnails_[i];
        D2D1_RECT_F rect = GetThumbnailRect(i);

        // Check if visible on screen
        if (rect.right < x_ || rect.left > x_ + width_) {
            continue;
        }

        if (thumb.bitmap) {
            // Render thumbnail
            D2D1_RECT_F srcRect = {
                0.0f,
                0.0f,
                static_cast<float>(thumb.width),
                static_cast<float>(thumb.height)
            };

            context->DrawBitmap(
                thumb.bitmap.Get(),
                rect,
                1.0f,
                D2D1_INTERPOLATION_MODE_LINEAR,
                srcRect,
                nullptr
            );
        } else if (thumb.isLoading) {
            // Render loading indicator
            RenderLoadingIndicator(context, i);
        } else if (thumb.isFailed) {
            // Render failed indicator
            context->FillRectangle(rect, failedBrush_.Get());
        }
    }
}

void ThumbnailStrip::RenderSelection(ID2D1DeviceContext* context, size_t index)
{
    if (index >= thumbnails_.size()) {
        return;
    }

    D2D1_RECT_F rect = GetThumbnailRect(index);

    // Expand rect for border
    float borderThickness = 3.0f;
    D2D1_RECT_F borderRect = D2D1::RectF(
        rect.left - borderThickness,
        rect.top - borderThickness,
        rect.right + borderThickness,
        rect.bottom + borderThickness
    );

    context->DrawRectangle(
        borderRect,
        selectedBorderBrush_.Get(),
        borderThickness
    );
}

void ThumbnailStrip::RenderLoadingIndicator(ID2D1DeviceContext* context, size_t index)
{
    D2D1_RECT_F rect = GetThumbnailRect(index);

    // Draw spinning loader
    float centerX = rect.left + rect.right / 2.0f;
    float centerY = rect.top + rect.bottom / 2.0f;
    float radius = std::min(rect.right - rect.left, rect.bottom - rect.top) / 4.0f;

    ULONGLONG time = GetTickCount64();
    float angle = static_cast<float>(time % 1000) / 1000.0f * 360.0f;

    // Simple loading circle
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(centerX, centerY), radius, radius);

    // TODO: Draw arc or spinner animation
    context->DrawEllipse(ellipse, loadingBrush_.Get(), 2.0f);
}

} // namespace UI
} // namespace UltraImageViewer
