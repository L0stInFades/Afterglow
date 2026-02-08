#pragma once

#include <d2d1.h>

namespace UltraImageViewer {
namespace UI {
namespace Theme {

    // Background
    constexpr D2D1_COLOR_F Background     = {0.07f, 0.07f, 0.07f, 1.0f};
    constexpr D2D1_COLOR_F Surface        = {0.12f, 0.12f, 0.12f, 1.0f};
    constexpr D2D1_COLOR_F SurfaceHover   = {0.18f, 0.18f, 0.18f, 1.0f};

    // Text
    constexpr D2D1_COLOR_F TextPrimary    = {0.95f, 0.95f, 0.95f, 1.0f};
    constexpr D2D1_COLOR_F TextSecondary  = {0.60f, 0.60f, 0.60f, 1.0f};

    // Accent
    constexpr D2D1_COLOR_F Accent         = {0.04f, 0.52f, 1.0f, 1.0f};

    // Viewer
    constexpr D2D1_COLOR_F ViewerBg       = {0.0f, 0.0f, 0.0f, 1.0f};
    constexpr D2D1_COLOR_F ViewerOverlay  = {0.0f, 0.0f, 0.0f, 0.5f};

    // Dimensions
    constexpr float ThumbnailGap = 2.0f;
    constexpr float ThumbnailCornerRadius = 4.0f;
    constexpr float TransitionCornerRadius = 8.0f;

    // Gallery
    constexpr float MinCellSize = 220.0f;
    constexpr float MaxCellSize = 400.0f;
    constexpr float GalleryPadding = 16.0f;
    constexpr float GalleryHeaderHeight = 72.0f;
    constexpr float SectionHeaderHeight = 36.0f;
    constexpr float SectionGap = 4.0f;

    // Animation configs (stiffness, damping, mass, restThreshold)
    // Snappy for transitions
    constexpr float TransitionStiffness = 300.0f;
    constexpr float TransitionDamping = 25.0f;
    // Smooth for scrolling
    constexpr float ScrollStiffness = 150.0f;
    constexpr float ScrollDamping = 22.0f;
    // Bouncy for rubber band
    constexpr float RubberBandStiffness = 400.0f;
    constexpr float RubberBandDamping = 30.0f;

} // namespace Theme
} // namespace UI
} // namespace UltraImageViewer
