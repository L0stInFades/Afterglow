#pragma once

#include "SpringAnimation.hpp"
#include <vector>
#include <memory>
#include <functional>

namespace UltraImageViewer {
namespace Animation {

class AnimationEngine {
public:
    AnimationEngine();
    ~AnimationEngine();

    void Update(float deltaTime);
    bool HasActiveAnimations() const;

    // Create a managed spring animation
    SpringAnimation* CreateSpring(const SpringConfig& config = {});
    void RemoveAnimation(SpringAnimation* anim);

    // Convenience: animate a value with callback
    using ValueCallback = std::function<void(float)>;
    using CompletionCallback = std::function<void()>;

    void AnimateValue(float from, float to, const SpringConfig& config,
                      ValueCallback onUpdate, CompletionCallback onComplete = nullptr);

    // Clear all animations
    void Clear();

private:
    struct ManagedSpring {
        std::unique_ptr<SpringAnimation> animation;
        ValueCallback onUpdate;
        CompletionCallback onComplete;
        bool isManaged = false;  // true if created via AnimateValue
    };

    std::vector<ManagedSpring> springs_;
};

} // namespace Animation
} // namespace UltraImageViewer
