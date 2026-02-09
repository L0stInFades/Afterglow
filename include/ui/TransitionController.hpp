#pragma once

#include <d2d1.h>
#include <wrl/client.h>
#include <functional>
#include "../animation/SpringAnimation.hpp"
#include "../animation/AnimationEngine.hpp"
#include "../rendering/Direct2DRenderer.hpp"

namespace UltraImageViewer {
namespace UI {

class TransitionController {
public:
    TransitionController();
    ~TransitionController();

    void Initialize(Animation::AnimationEngine* engine);

    // Gallery -> Viewer
    void StartGalleryToViewer(
        Microsoft::WRL::ComPtr<ID2D1Bitmap> thumbnail,
        D2D1_RECT_F fromRect,
        D2D1_RECT_F toRect,
        std::function<void()> onComplete
    );

    // Viewer -> Gallery
    void StartViewerToGallery(
        Microsoft::WRL::ComPtr<ID2D1Bitmap> image,
        D2D1_RECT_F fromRect,
        D2D1_RECT_F toRect,
        float initialOpacity,
        std::function<void()> onComplete
    );

    bool IsActive() const { return active_; }
    void Render(Rendering::Direct2DRenderer* renderer);
    void Update(float deltaTime);

private:
    Animation::SpringAnimation rectX_;
    Animation::SpringAnimation rectY_;
    Animation::SpringAnimation rectW_;
    Animation::SpringAnimation rectH_;
    Animation::SpringAnimation opacity_;
    Animation::SpringAnimation cornerRadius_;

    Microsoft::WRL::ComPtr<ID2D1Bitmap> transitionBitmap_;
    std::function<void()> onComplete_;
    bool active_ = false;
    bool isForward_ = true;  // true = gallery->viewer, false = viewer->gallery

    Animation::AnimationEngine* engine_ = nullptr;
};

} // namespace UI
} // namespace UltraImageViewer
