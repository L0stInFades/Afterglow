#include "ui/ViewManager.hpp"
#include "ui/Theme.hpp"

namespace UltraImageViewer {
namespace UI {

ViewManager::ViewManager() = default;
ViewManager::~ViewManager() = default;

void ViewManager::Initialize(Rendering::Direct2DRenderer* renderer,
                              Animation::AnimationEngine* engine,
                              Core::ImagePipeline* pipeline)
{
    animEngine_ = engine;
    pipeline_ = pipeline;

    galleryView_.Initialize(renderer, pipeline, engine);
    imageViewer_.Initialize(renderer, pipeline, engine);
    transition_.Initialize(engine);

    // Set up dismiss callback: viewer -> gallery
    imageViewer_.SetDismissCallback([this](size_t index) {
        TransitionToGallery();
    });
}

void ViewManager::SetState(ViewState state)
{
    state_ = state;
    needsRender_ = true;
}

void ViewManager::Render(Rendering::Direct2DRenderer* renderer)
{
    if (!renderer) return;

    switch (state_) {
        case ViewState::Gallery:
            galleryView_.Render(renderer);
            break;

        case ViewState::Viewer:
            imageViewer_.Render(renderer);
            break;

        case ViewState::Transition:
            // Render the background view
            if (pendingState_ == ViewState::Viewer) {
                galleryView_.Render(renderer);
            } else {
                imageViewer_.Render(renderer);
            }
            // Render transition on top
            transition_.Render(renderer);
            break;
    }

    needsRender_ = false;
}

void ViewManager::Update(float deltaTime)
{
    switch (state_) {
        case ViewState::Gallery:
            galleryView_.Update(deltaTime);
            if (!galleryView_.GetImages().empty()) {
                needsRender_ = true;  // Always re-render gallery for scroll animations
            }
            break;

        case ViewState::Viewer:
            imageViewer_.Update(deltaTime);
            needsRender_ = true;
            break;

        case ViewState::Transition:
            transition_.Update(deltaTime);
            needsRender_ = true;
            break;
    }
}

void ViewManager::TransitionToViewer(size_t imageIndex, D2D1_RECT_F fromRect)
{
    if (state_ != ViewState::Gallery) return;

    auto& images = galleryView_.GetImages();
    if (imageIndex >= images.size()) return;

    // Set up viewer
    imageViewer_.SetImages(images, imageIndex);
    imageViewer_.SetViewSize(viewWidth_, viewHeight_);

    // Get the target rect (full-screen fit)
    D2D1_RECT_F toRect = imageViewer_.GetCurrentImageRect();

    // Get thumbnail for transition
    auto thumbnail = pipeline_ ? pipeline_->GetThumbnail(images[imageIndex]) : nullptr;

    if (thumbnail) {
        pendingState_ = ViewState::Viewer;
        state_ = ViewState::Transition;

        transition_.StartGalleryToViewer(thumbnail, fromRect, toRect, [this]() {
            state_ = ViewState::Viewer;
            needsRender_ = true;
        });
    } else {
        // No thumbnail - instant transition
        state_ = ViewState::Viewer;
    }

    needsRender_ = true;
}

void ViewManager::TransitionToGallery()
{
    if (state_ != ViewState::Viewer) return;

    size_t currentIndex = imageViewer_.GetCurrentIndex();
    D2D1_RECT_F fromRect = imageViewer_.GetCurrentImageRect();

    // Get the gallery cell rect for the current image
    auto cellRect = galleryView_.GetCellScreenRect(currentIndex);
    D2D1_RECT_F toRect = cellRect.value_or(D2D1::RectF(
        viewWidth_ * 0.5f - 50.0f, viewHeight_ * 0.5f - 50.0f,
        viewWidth_ * 0.5f + 50.0f, viewHeight_ * 0.5f + 50.0f));

    auto& images = imageViewer_.GetImages();
    auto bitmap = pipeline_ ? pipeline_->GetThumbnail(images[currentIndex]) : nullptr;

    if (bitmap) {
        pendingState_ = ViewState::Gallery;
        state_ = ViewState::Transition;

        transition_.StartViewerToGallery(bitmap, fromRect, toRect, [this]() {
            state_ = ViewState::Gallery;
            needsRender_ = true;
        });
    } else {
        state_ = ViewState::Gallery;
    }

    needsRender_ = true;
}

void ViewManager::OnMouseWheel(float delta, float x, float y)
{
    switch (state_) {
        case ViewState::Gallery:
            galleryView_.OnMouseWheel(delta);
            break;
        case ViewState::Viewer:
            imageViewer_.OnMouseWheel(delta, x, y);
            break;
        default:
            break;
    }
    needsRender_ = true;
}

void ViewManager::OnKeyDown(UINT key)
{
    switch (state_) {
        case ViewState::Gallery:
            // Ctrl+O is handled by Application
            break;
        case ViewState::Viewer:
            imageViewer_.OnKeyDown(key);
            break;
        default:
            break;
    }
    needsRender_ = true;
}

void ViewManager::OnMouseDown(float x, float y)
{
    switch (state_) {
        case ViewState::Gallery:
            galleryView_.OnMouseDown(x, y);
            break;
        case ViewState::Viewer:
            imageViewer_.OnMouseDown(x, y);
            break;
        default:
            break;
    }
    needsRender_ = true;
}

void ViewManager::OnMouseMove(float x, float y)
{
    switch (state_) {
        case ViewState::Gallery:
            galleryView_.OnMouseMove(x, y);
            break;
        case ViewState::Viewer:
            imageViewer_.OnMouseMove(x, y);
            break;
        default:
            break;
    }
    needsRender_ = true;
}

void ViewManager::OnMouseUp(float x, float y)
{
    switch (state_) {
        case ViewState::Gallery: {
            galleryView_.OnMouseUp(x, y);
            // Check if a cell was clicked (not dragged)
            if (!galleryView_.WasDragging()) {
                auto hit = galleryView_.HitTest(x, y);
                if (hit.has_value()) {
                    TransitionToViewer(hit->index, hit->rect);
                }
            }
            break;
        }
        case ViewState::Viewer:
            imageViewer_.OnMouseUp(x, y);
            break;
        default:
            break;
    }
    needsRender_ = true;
}

void ViewManager::OnGesture(const GestureEventArgs& args)
{
    // Route gesture events based on type
    switch (args.type) {
        case GestureType::Zoom:
            if (state_ == ViewState::Viewer) {
                imageViewer_.OnMouseWheel(args.delta, args.x, args.y);
            }
            break;
        case GestureType::DoubleTap:
            // Handled in mouseUp via double-click detection
            break;
        default:
            break;
    }
    needsRender_ = true;
}

void ViewManager::SetViewSize(float width, float height)
{
    viewWidth_ = width;
    viewHeight_ = height;
    galleryView_.SetViewSize(width, height);
    imageViewer_.SetViewSize(width, height);
    needsRender_ = true;
}

} // namespace UI
} // namespace UltraImageViewer
