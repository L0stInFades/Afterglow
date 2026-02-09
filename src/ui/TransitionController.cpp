#include "ui/TransitionController.hpp"
#include "ui/Theme.hpp"
#include <cmath>

using Microsoft::WRL::ComPtr;

namespace UltraImageViewer {
namespace UI {

using Animation::SpringConfig;

static constexpr SpringConfig kTransitionSpring{
    Theme::TransitionStiffness, Theme::TransitionDamping, 1.0f, 0.5f};

TransitionController::TransitionController()
    : rectX_(kTransitionSpring)
    , rectY_(kTransitionSpring)
    , rectW_(kTransitionSpring)
    , rectH_(kTransitionSpring)
    , opacity_(kTransitionSpring)
    , cornerRadius_(kTransitionSpring)
{
}

TransitionController::~TransitionController() = default;

void TransitionController::Initialize(Animation::AnimationEngine* engine)
{
    engine_ = engine;
}

void TransitionController::StartGalleryToViewer(
    Microsoft::WRL::ComPtr<ID2D1Bitmap> thumbnail,
    D2D1_RECT_F fromRect,
    D2D1_RECT_F toRect,
    std::function<void()> onComplete)
{
    transitionBitmap_ = thumbnail;
    onComplete_ = std::move(onComplete);
    active_ = true;
    isForward_ = true;

    rectX_.SetValue(fromRect.left);
    rectX_.SetTarget(toRect.left);
    rectY_.SetValue(fromRect.top);
    rectY_.SetTarget(toRect.top);
    rectW_.SetValue(fromRect.right - fromRect.left);
    rectW_.SetTarget(toRect.right - toRect.left);
    rectH_.SetValue(fromRect.bottom - fromRect.top);
    rectH_.SetTarget(toRect.bottom - toRect.top);

    opacity_.SetValue(0.0f);
    opacity_.SetTarget(1.0f);

    cornerRadius_.SetValue(Theme::ThumbnailCornerRadius);
    cornerRadius_.SetTarget(0.0f);
}

void TransitionController::StartViewerToGallery(
    Microsoft::WRL::ComPtr<ID2D1Bitmap> image,
    D2D1_RECT_F fromRect,
    D2D1_RECT_F toRect,
    float initialOpacity,
    std::function<void()> onComplete)
{
    transitionBitmap_ = image;
    onComplete_ = std::move(onComplete);
    active_ = true;
    isForward_ = false;

    rectX_.SetValue(fromRect.left);
    rectX_.SetTarget(toRect.left);
    rectY_.SetValue(fromRect.top);
    rectY_.SetTarget(toRect.top);
    rectW_.SetValue(fromRect.right - fromRect.left);
    rectW_.SetTarget(toRect.right - toRect.left);
    rectH_.SetValue(fromRect.bottom - fromRect.top);
    rectH_.SetTarget(toRect.bottom - toRect.top);

    // Start from current overlay opacity (seamless handoff from viewer)
    opacity_.SetValue(initialOpacity);
    opacity_.SetTarget(0.0f);

    cornerRadius_.SetValue(0.0f);
    cornerRadius_.SetTarget(Theme::ThumbnailCornerRadius);
}

void TransitionController::Render(Rendering::Direct2DRenderer* renderer)
{
    if (!active_ || !renderer || !transitionBitmap_) return;

    auto* ctx = renderer->GetContext();
    if (!ctx) return;

    float x = rectX_.GetValue();
    float y = rectY_.GetValue();
    float w = rectW_.GetValue();
    float h = rectH_.GetValue();
    float alpha = opacity_.GetValue();
    float radius = cornerRadius_.GetValue();

    // Draw background overlay
    D2D1_COLOR_F bgColor = {0.0f, 0.0f, 0.0f, alpha};
    auto bgBrush = renderer->CreateBrush(bgColor);
    if (bgBrush) {
        FLOAT dpiX, dpiY;
        ctx->GetDpi(&dpiX, &dpiY);
        // Get render target size
        auto rtSize = ctx->GetSize();
        D2D1_RECT_F fullRect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);
        ctx->FillRectangle(fullRect, bgBrush.Get());
    }

    // Draw transitioning image with rounded corners
    D2D1_RECT_F destRect = D2D1::RectF(x, y, x + w, y + h);

    if (radius > 0.1f) {
        D2D1_ROUNDED_RECT roundedRect = {destRect, radius, radius};
        ComPtr<ID2D1RoundedRectangleGeometry> clipGeometry;
        renderer->GetFactory()->CreateRoundedRectangleGeometry(roundedRect, &clipGeometry);

        if (clipGeometry) {
            D2D1_LAYER_PARAMETERS layerParams = D2D1::LayerParameters(
                D2D1::InfiniteRect(), clipGeometry.Get());
            ctx->PushLayer(layerParams, nullptr);
            renderer->DrawImage(transitionBitmap_.Get(), destRect, 1.0f);
            ctx->PopLayer();
        }
    } else {
        renderer->DrawImage(transitionBitmap_.Get(), destRect, 1.0f);
    }
}

void TransitionController::Update(float deltaTime)
{
    if (!active_) return;

    rectX_.Update(deltaTime);
    rectY_.Update(deltaTime);
    rectW_.Update(deltaTime);
    rectH_.Update(deltaTime);
    opacity_.Update(deltaTime);
    cornerRadius_.Update(deltaTime);

    // Check if all animations are complete
    if (rectX_.IsFinished() && rectY_.IsFinished() &&
        rectW_.IsFinished() && rectH_.IsFinished() &&
        opacity_.IsFinished() && cornerRadius_.IsFinished()) {
        active_ = false;
        if (onComplete_) {
            auto cb = std::move(onComplete_);
            cb();
        }
    }
}

} // namespace UI
} // namespace UltraImageViewer
