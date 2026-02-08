#pragma once

#include <d2d1_1.h>
#include <d2d1effects.h>
#include <wrl/client.h>
#include <vector>
#include <memory>

namespace UltraImageViewer {
namespace ImageProcessing {

/**
 * Filter types supported
 */
enum class FilterType {
    None,
    Blur,
    Sharpen,
    Brightness,
    Contrast,
    Saturation,
    Grayscale,
    Invert,
    Sepia,
    HueRotate,
    ColorMatrix,
    Opacity
};

/**
 * Filter parameters
 */
struct FilterParams {
    float amount;          // Effect intensity (0.0 - 1.0)
    float radius;          // For blur (pixels)
    float angle;           // For rotation (degrees)
    float red, green, blue; // For color adjustments
    float alpha;           // Opacity

    float matrix[5][4];    // For color matrix filter

    FilterParams() : amount(0.5f), radius(5.0f), angle(0.0f),
                     red(1.0f), green(1.0f), blue(1.0f), alpha(1.0f) {
        // Initialize color matrix to identity
        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 4; ++j) {
                matrix[i][j] = (i == j) ? 1.0f : 0.0f;
            }
        }
    }
};

/**
 * Single filter in the pipeline
 */
class Filter {
public:
    Filter(FilterType type, const FilterParams& params);
    ~Filter() = default;

    FilterType GetType() const { return type_; }
    const FilterParams& GetParams() const { return params_; }
    void SetParams(const FilterParams& params) { params_ = params; }

    // Create Direct2D effect
    Microsoft::WRL::ComPtr<ID2D1Effect> CreateEffect(ID2D1DeviceContext* context) const;

private:
    FilterType type_;
    FilterParams params_;
};

/**
 * Filter pipeline for real-time image effects
 * Chains multiple filters together for GPU-accelerated processing
 */
class FilterPipeline {
public:
    FilterPipeline();
    ~FilterPipeline();

    // Pipeline management
    void AddFilter(std::unique_ptr<Filter> filter);
    void RemoveFilter(size_t index);
    void ClearFilters();
    size_t GetFilterCount() const { return filters_.size(); }

    // Apply pipeline to image
    Microsoft::WRL::ComPtr<ID2D1Image> Apply(
        ID2D1DeviceContext* context,
        ID2D1Bitmap* inputBitmap);

    // Preview with specific filter settings (without committing)
    Microsoft::WRL::ComPtr<ID2D1Image> Preview(
        ID2D1DeviceContext* context,
        ID2D1Bitmap* inputBitmap,
        size_t filterIndex,
        const FilterParams& newParams);

    // Enable/disable pipeline
    void SetEnabled(bool enabled) { enabled_ = enabled; }
    bool IsEnabled() const { return enabled_; }

    // Preset filters
    static std::unique_ptr<Filter> CreateBlurFilter(float radius);
    static std::unique_ptr<Filter> CreateSharpenFilter(float amount);
    static std::unique_ptr<Filter> CreateBrightnessFilter(float amount);
    static std::unique_ptr<Filter> CreateContrastFilter(float amount);
    static std::unique_ptr<Filter> CreateGrayscaleFilter();
    static std::unique_ptr<Filter> CreateInvertFilter();
    static std::unique_ptr<Filter> CreateSepiaFilter();
    static std::unique_ptr<Filter> CreateColorMatrixFilter(const D2D1_MATRIX_5X4_F& matrix);

private:
    // Helper to create individual effects
    Microsoft::WRL::ComPtr<ID2D1Effect> CreateBlurEffect(
        ID2D1DeviceContext* context, const FilterParams& params) const;
    Microsoft::WRL::ComPtr<ID2D1Effect> CreateSharpenEffect(
        ID2D1DeviceContext* context, const FilterParams& params) const;
    Microsoft::WRL::ComPtr<ID2D1Effect> CreateBrightnessEffect(
        ID2D1DeviceContext* context, const FilterParams& params) const;
    Microsoft::WRL::ComPtr<ID2D1Effect> CreateContrastEffect(
        ID2D1DeviceContext* context, const FilterParams& params) const;
    Microsoft::WRL::ComPtr<ID2D1Effect> CreateGrayscaleEffect(
        ID2D1DeviceContext* context) const;
    Microsoft::WRL::ComPtr<ID2D1Effect> CreateInvertEffect(
        ID2D1DeviceContext* context) const;
    Microsoft::WRL::ComPtr<ID2D1Effect> CreateSepiaEffect(
        ID2D1DeviceContext* context) const;
    Microsoft::WRL::ComPtr<ID2D1Effect> CreateColorMatrixEffect(
        ID2D1DeviceContext* context, const FilterParams& params) const;

    std::vector<std::unique_ptr<Filter>> filters_;
    bool enabled_;
};

} // namespace ImageProcessing
} // namespace UltraImageViewer
