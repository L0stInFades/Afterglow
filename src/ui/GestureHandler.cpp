#include "ui/GestureHandler.hpp"
#include <windows.h>

namespace UltraImageViewer {
namespace UI {

GestureHandler::GestureHandler()
    : hwnd_(nullptr)
    , zoomSensitivity_(1.0f)
    , rotateSensitivity_(1.0f)
    , panSensitivity_(1.0f)
    , maxTapDelay_(500)
    , maxDoubleTapDelay_(300)
    , pressAndHoldDelay_(800)
    , isInitialized_(false)
    , lastZoomDistance_(0.0f)
    , lastRotationAngle_(0.0f)
    , lastTapTime_(0)
{
    // Initialize all callbacks to nullptr
    for (size_t i = 0; i < static_cast<size_t>(GestureType::Swipe) + 1; ++i) {
        callbacks_[i] = nullptr;
        enabledGestures_[i] = false;
    }

    // Enable common gestures by default
    enabledGestures_[static_cast<size_t>(GestureType::Zoom)] = true;
    enabledGestures_[static_cast<size_t>(GestureType::Pan)] = true;
    enabledGestures_[static_cast<size_t>(GestureType::Tap)] = true;
    enabledGestures_[static_cast<size_t>(GestureType::DoubleTap)] = true;
}

GestureHandler::~GestureHandler()
{
    if (hwnd_ && isInitialized_) {
        // Cleanup gesture configuration
        SetGestureConfig(hwnd_, 0, 0, nullptr, 0);
    }
}

bool GestureHandler::Initialize(HWND hwnd)
{
    hwnd_ = hwnd;

    // Configure Windows gesture support
    DWORD flags = GetGestureConfigFlags();

    GESTURECONFIG config[] = {
        { GID_ZOOM, flags & GC_ZOOM ? 0 : GC_ZOOM, flags & GC_ZOOM ? GC_ZOOM : 0 },
        { GID_PAN, flags & GC_PAN ? 0 : GC_PAN, flags & GC_PAN ? GC_PAN : 0 },
        { GID_ROTATE, flags & GC_ROTATE ? 0 : GC_ROTATE, flags & GC_ROTATE ? GC_ROTATE : 0 },
        { GID_TWOFINGERTAP, flags & GC_TWOFINGERTAP ? 0 : GC_TWOFINGERTAP, flags & GC_TWOFINGERTAP ? GC_TWOFINGERTAP : 0 },
        { GID_PRESSANDTAP, flags & GC_PRESSANDTAP ? 0 : GC_PRESSANDTAP, flags & GC_PRESSANDTAP ? GC_PRESSANDTAP : 0 }
    };

    BOOL result = SetGestureConfig(
        hwnd,
        0,
        sizeof(config) / sizeof(config[0]),
        config,
        sizeof(GESTURECONFIG)
    );

    isInitialized_ = (result != FALSE);

    return isInitialized_;
}

void GestureHandler::SetGestureCallback(GestureType type, GestureCallback callback)
{
    size_t index = static_cast<size_t>(type);
    if (index <= static_cast<size_t>(GestureType::Swipe)) {
        callbacks_[index] = std::move(callback);
    }
}

void GestureHandler::ClearCallbacks()
{
    for (size_t i = 0; i < static_cast<size_t>(GestureType::Swipe) + 1; ++i) {
        callbacks_[i] = nullptr;
    }
}

bool GestureHandler::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* result)
{
    if (!isInitialized_) {
        return false;
    }

    switch (msg) {
        case WM_GESTURE: {
            GESTUREINFO gi = {};
            gi.cbSize = sizeof(GESTUREINFO);

            if (GetGestureInfo((HGESTUREINFO)lParam, &gi)) {
                switch (gi.dwID) {
                    case GID_ZOOM:
                        OnZoom(gi);
                        break;

                    case GID_PAN:
                        OnPan(gi);
                        break;

                    case GID_ROTATE:
                        OnRotate(gi);
                        break;

                    case GID_TWOFINGERTAP:
                        OnTwoFingerTap(gi);
                        break;

                    case GID_PRESSANDTAP:
                        OnPressAndTap(gi);
                        break;
                }

                CloseGestureInfoHandle((HGESTUREINFO)lParam);
                *result = 0;
                return true;
            }
            break;
        }

        case WM_TOUCH: {
            // Handle raw touch input if needed
            // For now, we rely on WM_GESTURE
            break;
        }
    }

    return false;
}

void GestureHandler::EnableGesture(GestureType type, bool enable)
{
    size_t index = static_cast<size_t>(type);
    if (index <= static_cast<size_t>(GestureType::Swipe)) {
        enabledGestures_[index] = enable;

        // Reconfigure if initialized
        if (isInitialized_) {
            Initialize(hwnd_);
        }
    }
}

bool GestureHandler::IsGestureEnabled(GestureType type) const
{
    size_t index = static_cast<size_t>(type);
    return index <= static_cast<size_t>(GestureType::Swipe) ? enabledGestures_[index] : false;
}

DWORD GestureHandler::GetGestureConfigFlags() const
{
    DWORD flags = 0;

    // All zoom
    if (enabledGestures_[static_cast<size_t>(GestureType::Zoom)]) {
        flags |= GC_ZOOM;
    }

    // All pan (with inertia)
    if (enabledGestures_[static_cast<size_t>(GestureType::Pan)]) {
        flags |= GC_PAN;
    }

    // All rotate
    if (enabledGestures_[static_cast<size_t>(GestureType::Rotate)]) {
        flags |= GC_ROTATE;
    }

    // Two-finger tap
    if (enabledGestures_[static_cast<size_t>(GestureType::TwoFingerTap)]) {
        flags |= GC_TWOFINGERTAP;
    }

    // Press and tap
    if (enabledGestures_[static_cast<size_t>(GestureType::PressAndTap)]) {
        flags |= GC_PRESSANDTAP;
    }

    return flags;
}

void GestureHandler::OnZoom(const GESTUREINFO& gi)
{
    size_t index = static_cast<size_t>(GestureType::Zoom);
    if (!callbacks_[index]) {
        return;
    }

    // Calculate zoom factor
    double zoomFactor = ((double)gi.ullArguments) / 65536.0;
    float delta = static_cast<float>(zoomFactor);

    // Handle begin/end
    if (gi.dwFlags & GF_BEGIN) {
        lastZoomDistance_ = delta;
        return;
    }

    // Calculate actual zoom delta
    float zoomDelta = (delta - lastZoomDistance_) * zoomSensitivity_;
    lastZoomDistance_ = delta;

    GestureEventArgs args = CreateEventArgs(GestureType::Zoom, gi, zoomDelta);
    args.distance = delta;

    callbacks_[index](args);
}

void GestureHandler::OnPan(const GESTUREINFO& gi)
{
    size_t index = static_cast<size_t>(GestureType::Pan);
    if (!callbacks_[index]) {
        return;
    }

    // Extract pan deltas
    // ullArguments contains LOWORD: x delta, HIWORD: y delta
    LONG deltaX = LOWORD(gi.ullArguments);
    LONG deltaY = HIWORD(gi.ullArguments);

    float deltaXf = static_cast<float>(deltaX) * panSensitivity_;
    float deltaYf = static_cast<float>(deltaY) * panSensitivity_;

    GestureEventArgs args = CreateEventArgs(GestureType::Pan, gi, deltaXf + deltaYf);
    args.x = static_cast<float>(gi.ptsLocation.x);
    args.y = static_cast<float>(gi.ptsLocation.y);

    callbacks_[index](args);
}

void GestureHandler::OnRotate(const GESTUREINFO& gi)
{
    size_t index = static_cast<size_t>(GestureType::Rotate);
    if (!callbacks_[index]) {
        return;
    }

    // Calculate rotation angle
    double angle = ((double)gi.ullArguments) / 65536.0;
    float anglef = static_cast<float>(angle);

    // Handle begin/end
    if (gi.dwFlags & GF_BEGIN) {
        lastRotationAngle_ = anglef;
        return;
    }

    // Calculate rotation delta
    float rotationDelta = (anglef - lastRotationAngle_) * rotateSensitivity_;
    lastRotationAngle_ = anglef;

    GestureEventArgs args = CreateEventArgs(GestureType::Rotate, gi, rotationDelta);
    args.angle = anglef;

    callbacks_[index](args);
}

void GestureHandler::OnTap(const GESTUREINFO& gi)
{
    // Single tap is usually handled via WM_LBUTTONUP
    // But can be detected here if needed
}

void GestureHandler::OnTwoFingerTap(const GESTUREINFO& gi)
{
    size_t index = static_cast<size_t>(GestureType::TwoFingerTap);
    if (!callbacks_[index]) {
        return;
    }

    GestureEventArgs args = CreateEventArgs(GestureType::TwoFingerTap, gi);
    args.pointers = 2;

    callbacks_[index](args);
}

void GestureHandler::OnPressAndTap(const GESTUREINFO& gi)
{
    size_t index = static_cast<size_t>(GestureType::PressAndTap);
    if (!callbacks_[index]) {
        return;
    }

    GestureEventArgs args = CreateEventArgs(GestureType::PressAndTap, gi);
    args.pointers = 2;

    callbacks_[index](args);
}

GestureEventArgs GestureHandler::CreateEventArgs(GestureType type, const GESTUREINFO& gi, float delta)
{
    GestureEventArgs args;
    args.type = type;
    args.x = static_cast<float>(gi.ptsLocation.x);
    args.y = static_cast<float>(gi.ptsLocation.y);
    args.delta = delta;
    args.pointers = static_cast<UINT>(gi.dwFlags);
    args.timestamp = gi.ullArguments;

    return args;
}

} // namespace UI
} // namespace UltraImageViewer
