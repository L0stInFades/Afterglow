#pragma once

#include <windows.h>
#include <functional>
#include <cmath>

namespace UltraImageViewer {
namespace UI {

/**
 * Gesture types supported
 */
enum class GestureType {
    None,
    Tap,           // Single tap
    DoubleTap,     // Rapid double tap
    PressAndTap,   // Press with one finger, tap with another
    Zoom,          // Pinch to zoom
    Pan,           // Two-finger pan
    Rotate,        // Two-finger rotate
    PressAndHold,  // Long press
    TwoFingerTap,   // Two-finger tap
    Swipe          // Quick swipe
};

/**
 * Gesture event data
 */
struct GestureEventArgs {
    GestureType type;
    float x;              // Center X position
    float y;              // Center Y position
    float delta;          // Distance/angle change
    float distance;       // Current distance between fingers
    float angle;          // Rotation angle
    UINT pointers;        // Number of touch points
    ULONG_PTR timestamp;  // Event timestamp

    GestureEventArgs() : type(GestureType::None), x(0), y(0), delta(0),
                         distance(0), angle(0), pointers(0), timestamp(0) {}
};

/**
 * Callback function type for gesture events
 */
using GestureCallback = std::function<void(const GestureEventArgs&)>;

/**
 * Multi-touch gesture handler
 * Supports Windows 7+ touch gestures and pen input
 */
class GestureHandler {
public:
    GestureHandler();
    ~GestureHandler();

    // Initialize with window handle
    bool Initialize(HWND hwnd);

    // Register callbacks for different gesture types
    void SetGestureCallback(GestureType type, GestureCallback callback);
    void ClearCallbacks();

    // Message handling (call from window procedure)
    bool HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* result);

    // Enable/disable specific gestures
    void EnableGesture(GestureType type, bool enable = true);
    bool IsGestureEnabled(GestureType type) const;

    // Configuration
    void SetZoomSensitivity(float sensitivity) { zoomSensitivity_ = sensitivity; }
    void SetRotateSensitivity(float sensitivity) { rotateSensitivity_ = sensitivity; }
    void SetPanSensitivity(float sensitivity) { panSensitivity_ = sensitivity; }

    // Gesture configuration
    void SetMaxTapDelay(UINT milliseconds) { maxTapDelay_ = milliseconds; }
    void SetMaxDoubleTapDelay(UINT milliseconds) { maxDoubleTapDelay_ = milliseconds; }
    void SetPressAndHoldDelay(UINT milliseconds) { pressAndHoldDelay_ = milliseconds; }

private:
    // Gesture configuration helpers
    DWORD GetGestureConfigFlags() const;

    // Individual gesture handlers
    void OnZoom(const GESTUREINFO& gi);
    void OnPan(const GESTUREINFO& gi);
    void OnRotate(const GESTUREINFO& gi);
    void OnTap(const GESTUREINFO& gi);
    void OnTwoFingerTap(const GESTUREINFO& gi);
    void OnPressAndTap(const GESTUREINFO& gi);

    // Helper to convert gesture info to event args
    GestureEventArgs CreateEventArgs(GestureType type, const GESTUREINFO& gi, float delta = 0);

    // Window handle
    HWND hwnd_;

    // Callbacks
    GestureCallback callbacks_[static_cast<size_t>(GestureType::Swipe) + 1];

    // Enabled gestures
    bool enabledGestures_[static_cast<size_t>(GestureType::Swipe) + 1];

    // Sensitivity settings
    float zoomSensitivity_;
    float rotateSensitivity_;
    float panSensitivity_;

    // Timing settings
    UINT maxTapDelay_;
    UINT maxDoubleTapDelay_;
    UINT pressAndHoldDelay_;

    // State tracking
    bool isInitialized_;
    float lastZoomDistance_;
    float lastRotationAngle_;
    ULONGLONG lastTapTime_;
};

} // namespace UI
} // namespace UltraImageViewer
