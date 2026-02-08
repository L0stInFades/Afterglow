#include "image_processing/FilterPipeline.hpp"

namespace UltraImageViewer {
namespace ImageProcessing {

// Filter implementation
Filter::Filter(FilterType type, const FilterParams& params)
    : type_(type)
    , params_(params)
{
}

Microsoft::WRL::ComPtr<ID2D1Effect> Filter::CreateEffect(ID2D1DeviceContext* context) const
{
    // This is a factory method - the actual creation is done by FilterPipeline
    // which has access to all the helper methods
    return nullptr;
}

// FilterPipeline implementation
FilterPipeline::FilterPipeline()
    : enabled_(true)
{
}

FilterPipeline::~FilterPipeline() = default;

void FilterPipeline::AddFilter(std::unique_ptr<Filter> filter)
{
    filters_.push_back(std::move(filter));
}

void FilterPipeline::RemoveFilter(size_t index)
{
    if (index < filters_.size()) {
        filters_.erase(filters_.begin() + index);
    }
}

void FilterPipeline::ClearFilters()
{
    filters_.clear();
}

Microsoft::WRL::ComPtr<ID2D1Image> FilterPipeline::Apply(
    ID2D1DeviceContext* context,
    ID2D1Bitmap* inputBitmap)
{
    if (!enabled_ || filters_.empty()) {
        // Return input as-is
        Microsoft::WRL::ComPtr<ID2D1Image> result;
        inputBitmap->QueryInterface(result.GetAddressOf());
        return result;
    }

    Microsoft::WRL::ComPtr<ID2D1Image> currentOutput;
    inputBitmap->QueryInterface(currentOutput.GetAddressOf());

    // Chain effects together
    for (const auto& filter : filters_) {
        Microsoft::WRL::ComPtr<ID2D1Effect> effect;

        // Create appropriate effect based on filter type
        switch (filter->GetType()) {
            case FilterType::Blur:
                effect = CreateBlurEffect(context, filter->GetParams());
                break;

            case FilterType::Sharpen:
                effect = CreateSharpenEffect(context, filter->GetParams());
                break;

            case FilterType::Brightness:
                effect = CreateBrightnessEffect(context, filter->GetParams());
                break;

            case FilterType::Contrast:
                effect = CreateContrastEffect(context, filter->GetParams());
                break;

            case FilterType::Grayscale:
                effect = CreateGrayscaleEffect(context);
                break;

            case FilterType::Invert:
                effect = CreateInvertEffect(context);
                break;

            case FilterType::Sepia:
                effect = CreateSepiaEffect(context);
                break;

            case FilterType::ColorMatrix:
                effect = CreateColorMatrixEffect(context, filter->GetParams());
                break;

            default:
                // Unknown filter, skip
                continue;
        }

        if (effect) {
            // Set input and update output
            effect->SetInput(0, currentOutput.Get());
            currentOutput.Reset();
            effect->QueryInterface(currentOutput.GetAddressOf());
        }
    }

    return currentOutput;
}

Microsoft::WRL::ComPtr<ID2D1Image> FilterPipeline::Preview(
    ID2D1DeviceContext* context,
    ID2D1Bitmap* inputBitmap,
    size_t filterIndex,
    const FilterParams& newParams)
{
    if (filterIndex >= filters_.size()) {
        return Apply(context, inputBitmap);
    }

    // Temporarily modify filter params
    FilterParams originalParams = filters_[filterIndex]->GetParams();
    filters_[filterIndex]->SetParams(newParams);

    // Apply pipeline
    auto result = Apply(context, inputBitmap);

    // Restore original params
    filters_[filterIndex]->SetParams(originalParams);

    return result;
}

// Preset filters
std::unique_ptr<Filter> FilterPipeline::CreateBlurFilter(float radius)
{
    FilterParams params;
    params.radius = radius;
    return std::make_unique<Filter>(FilterType::Blur, params);
}

std::unique_ptr<Filter> FilterPipeline::CreateSharpenFilter(float amount)
{
    FilterParams params;
    params.amount = amount;
    return std::make_unique<Filter>(FilterType::Sharpen, params);
}

std::unique_ptr<Filter> FilterPipeline::CreateBrightnessFilter(float amount)
{
    FilterParams params;
    params.amount = amount;
    return std::make_unique<Filter>(FilterType::Brightness, params);
}

std::unique_ptr<Filter> FilterPipeline::CreateContrastFilter(float amount)
{
    FilterParams params;
    params.amount = amount;
    return std::make_unique<Filter>(FilterType::Contrast, params);
}

std::unique_ptr<Filter> FilterPipeline::CreateGrayscaleFilter()
{
    FilterParams params;
    return std::make_unique<Filter>(FilterType::Grayscale, params);
}

std::unique_ptr<Filter> FilterPipeline::CreateInvertFilter()
{
    FilterParams params;
    return std::make_unique<Filter>(FilterType::Invert, params);
}

std::unique_ptr<Filter> FilterPipeline::CreateSepiaFilter()
{
    FilterParams params;
    return std::make_unique<Filter>(FilterType::Sepia, params);
}

std::unique_ptr<Filter> FilterPipeline::CreateColorMatrixFilter(const D2D1_MATRIX_5X4_F& matrix)
{
    FilterParams params;
    memcpy(params.matrix, matrix.m, sizeof(params.matrix));
    return std::make_unique<Filter>(FilterType::ColorMatrix, params);
}

// Effect creation helpers
Microsoft::WRL::ComPtr<ID2D1Effect> FilterPipeline::CreateBlurEffect(
    ID2D1DeviceContext* context, const FilterParams& params) const
{
    Microsoft::WRL::ComPtr<ID2D1Effect> effect;
    HRESULT hr = context->CreateEffect(CLSID_D2D1GaussianBlur, &effect);

    if (SUCCEEDED(hr)) {
        effect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, params.radius);
        effect->SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION, D2D1_GAUSSIANBLUR_OPTIMIZATION_BALANCED);
    }

    return effect;
}

Microsoft::WRL::ComPtr<ID2D1Effect> FilterPipeline::CreateSharpenEffect(
    ID2D1DeviceContext* context, const FilterParams& params) const
{
    Microsoft::WRL::ComPtr<ID2D1Effect> effect;
    HRESULT hr = context->CreateEffect(CLSID_D2D1Sharpen, &effect);

    if (SUCCEEDED(hr)) {
        effect->SetValue(D2D1_SHARPEN_PROP_SHARPNESS, params.amount);
        effect->SetValue(D2D1_SHARPEN_PROP_THRESHOLD, 0.5f);
    }

    return effect;
}

Microsoft::WRL::ComPtr<ID2D1Effect> FilterPipeline::CreateBrightnessEffect(
    ID2D1DeviceContext* context, const FilterParams& params) const
{
    // Brightness is implemented using color matrix
    // Brightness: 0.0 = black, 0.5 = normal, 1.0 = white
    D2D1_MATRIX_5X4_F matrix = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        (params.amount - 0.5f), (params.amount - 0.5f), (params.amount - 0.5f), 0.0f
    };

    Microsoft::WRL::ComPtr<ID2D1Effect> effect;
    HRESULT hr = context->CreateEffect(CLSID_D2D1ColorMatrix, &effect);

    if (SUCCEEDED(hr)) {
        effect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);
    }

    return effect;
}

Microsoft::WRL::ComPtr<ID2D1Effect> FilterPipeline::CreateContrastEffect(
    ID2D1DeviceContext* context, const FilterParams& params) const
{
    // Contrast: 0.0 = gray, 0.5 = normal, 1.0 = high contrast
    float contrast = (params.amount * 2.0f); // Map 0.5->1.0

    D2D1_MATRIX_5X4_F matrix = {
        contrast, 0.0f, 0.0f, 0.0f,
        0.0f, contrast, 0.0f, 0.0f,
        0.0f, 0.0f, contrast, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        0.5f * (1.0f - contrast), 0.5f * (1.0f - contrast), 0.5f * (1.0f - contrast), 0.0f
    };

    Microsoft::WRL::ComPtr<ID2D1Effect> effect;
    HRESULT hr = context->CreateEffect(CLSID_D2D1ColorMatrix, &effect);

    if (SUCCEEDED(hr)) {
        effect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);
    }

    return effect;
}

Microsoft::WRL::ComPtr<ID2D1Effect> FilterPipeline::CreateGrayscaleEffect(
    ID2D1DeviceContext* context) const
{
    // Standard grayscale weights (luminance method)
    D2D1_MATRIX_5X4_F matrix = {
        0.299f, 0.299f, 0.299f, 0.0f,
        0.587f, 0.587f, 0.587f, 0.0f,
        0.114f, 0.114f, 0.114f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    };

    Microsoft::WRL::ComPtr<ID2D1Effect> effect;
    HRESULT hr = context->CreateEffect(CLSID_D2D1ColorMatrix, &effect);

    if (SUCCEEDED(hr)) {
        effect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);
    }

    return effect;
}

Microsoft::WRL::ComPtr<ID2D1Effect> FilterPipeline::CreateInvertEffect(
    ID2D1DeviceContext* context) const
{
    D2D1_MATRIX_5X4_F matrix = {
        -1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f
    };

    Microsoft::WRL::ComPtr<ID2D1Effect> effect;
    HRESULT hr = context->CreateEffect(CLSID_D2D1ColorMatrix, &effect);

    if (SUCCEEDED(hr)) {
        effect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);
    }

    return effect;
}

Microsoft::WRL::ComPtr<ID2D1Effect> FilterPipeline::CreateSepiaEffect(
    ID2D1DeviceContext* context) const
{
    // Sepia tone matrix
    D2D1_MATRIX_5X4_F matrix = {
        0.393f, 0.349f, 0.272f, 0.0f,
        0.769f, 0.686f, 0.534f, 0.0f,
        0.189f, 0.168f, 0.131f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    };

    Microsoft::WRL::ComPtr<ID2D1Effect> effect;
    HRESULT hr = context->CreateEffect(CLSID_D2D1ColorMatrix, &effect);

    if (SUCCEEDED(hr)) {
        effect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);
    }

    return effect;
}

Microsoft::WRL::ComPtr<ID2D1Effect> FilterPipeline::CreateColorMatrixEffect(
    ID2D1DeviceContext* context, const FilterParams& params) const
{
    D2D1_MATRIX_5X4_F matrix;
    memcpy(&matrix, params.matrix, sizeof(matrix));

    Microsoft::WRL::ComPtr<ID2D1Effect> effect;
    HRESULT hr = context->CreateEffect(CLSID_D2D1ColorMatrix, &effect);

    if (SUCCEEDED(hr)) {
        effect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);
    }

    return effect;
}

} // namespace ImageProcessing
} // namespace UltraImageViewer
