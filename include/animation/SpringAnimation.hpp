#pragma once

#include <algorithm>
#include <cmath>

namespace UltraImageViewer {
namespace Animation {

struct SpringConfig {
    float stiffness = 200.0f;
    float damping = 20.0f;
    float mass = 1.0f;
    float restThreshold = 0.5f;
};

class SpringAnimation {
public:
    explicit SpringAnimation(const SpringConfig& config = {});

    void SetTarget(float target);
    void SetValue(float value);
    void SetVelocity(float velocity);
    void SetConfig(const SpringConfig& config);

    float Update(float deltaTime);
    bool IsFinished() const;

    float GetValue() const { return value_; }
    float GetTarget() const { return target_; }
    float GetVelocity() const { return velocity_; }
    const SpringConfig& GetConfig() const { return config_; }

    // Snap to target immediately
    void SnapToTarget();

private:
    float value_ = 0.0f;
    float velocity_ = 0.0f;
    float target_ = 0.0f;
    SpringConfig config_;
    bool finished_ = true;
};

class SpringAnimation2D {
public:
    explicit SpringAnimation2D(const SpringConfig& config = {});

    void SetTarget(float tx, float ty);
    void SetValue(float vx, float vy);
    void SetVelocity(float vx, float vy);
    void SetConfig(const SpringConfig& config);

    void Update(float deltaTime);
    bool IsFinished() const;

    float GetX() const { return x_.GetValue(); }
    float GetY() const { return y_.GetValue(); }
    float GetTargetX() const { return x_.GetTarget(); }
    float GetTargetY() const { return y_.GetTarget(); }

    SpringAnimation& X() { return x_; }
    SpringAnimation& Y() { return y_; }

    void SnapToTarget();

private:
    SpringAnimation x_;
    SpringAnimation y_;
};

} // namespace Animation
} // namespace UltraImageViewer
