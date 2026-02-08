#include "animation/SpringAnimation.hpp"

namespace UltraImageViewer {
namespace Animation {

SpringAnimation::SpringAnimation(const SpringConfig& config)
    : config_(config)
{
}

void SpringAnimation::SetTarget(float target)
{
    target_ = target;
    finished_ = false;
}

void SpringAnimation::SetValue(float value)
{
    value_ = value;
    finished_ = false;
}

void SpringAnimation::SetVelocity(float velocity)
{
    velocity_ = velocity;
    finished_ = false;
}

void SpringAnimation::SetConfig(const SpringConfig& config)
{
    config_ = config;
}

float SpringAnimation::Update(float deltaTime)
{
    if (finished_) {
        return value_;
    }

    // Spring physics: F = -kx - cv
    // where k = stiffness, c = damping, x = displacement, v = velocity
    float displacement = value_ - target_;
    float springForce = -config_.stiffness * displacement;
    float dampingForce = -config_.damping * velocity_;
    float acceleration = (springForce + dampingForce) / config_.mass;

    // Semi-implicit Euler integration (more stable than explicit Euler)
    velocity_ += acceleration * deltaTime;
    value_ += velocity_ * deltaTime;

    // Check if animation has settled
    if (std::abs(velocity_) < config_.restThreshold &&
        std::abs(value_ - target_) < config_.restThreshold) {
        value_ = target_;
        velocity_ = 0.0f;
        finished_ = true;
    }

    return value_;
}

bool SpringAnimation::IsFinished() const
{
    return finished_;
}

void SpringAnimation::SnapToTarget()
{
    value_ = target_;
    velocity_ = 0.0f;
    finished_ = true;
}

// SpringAnimation2D

SpringAnimation2D::SpringAnimation2D(const SpringConfig& config)
    : x_(config), y_(config)
{
}

void SpringAnimation2D::SetTarget(float tx, float ty)
{
    x_.SetTarget(tx);
    y_.SetTarget(ty);
}

void SpringAnimation2D::SetValue(float vx, float vy)
{
    x_.SetValue(vx);
    y_.SetValue(vy);
}

void SpringAnimation2D::SetVelocity(float vx, float vy)
{
    x_.SetVelocity(vx);
    y_.SetVelocity(vy);
}

void SpringAnimation2D::SetConfig(const SpringConfig& config)
{
    x_.SetConfig(config);
    y_.SetConfig(config);
}

void SpringAnimation2D::Update(float deltaTime)
{
    x_.Update(deltaTime);
    y_.Update(deltaTime);
}

bool SpringAnimation2D::IsFinished() const
{
    return x_.IsFinished() && y_.IsFinished();
}

void SpringAnimation2D::SnapToTarget()
{
    x_.SnapToTarget();
    y_.SnapToTarget();
}

} // namespace Animation
} // namespace UltraImageViewer
