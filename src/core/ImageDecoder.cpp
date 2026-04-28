#include "core/ImageDecoder.hpp"
#include "core/SimdUtils.hpp"
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <limits>
#include <new>
#include <thread>

namespace {
bool CalculateBgraLayout(uint32_t width, uint32_t height, size_t& dataSize, UINT& stride, UINT& bufferSize)
{
    if (width == 0 || height == 0) {
        return false;
    }

    constexpr uint64_t kBytesPerPixel = 4;
    uint64_t stride64 = static_cast<uint64_t>(width) * kBytesPerPixel;
    uint64_t size64 = stride64 * static_cast<uint64_t>(height);

    if (stride64 > std::numeric_limits<UINT>::max() ||
        size64 > std::numeric_limits<UINT>::max() ||
        size64 > std::numeric_limits<size_t>::max()) {
        return false;
    }

    stride = static_cast<UINT>(stride64);
    bufferSize = static_cast<UINT>(size64);
    dataSize = static_cast<size_t>(size64);
    return true;
}

uint32_t ScaleToMax(uint32_t value, uint32_t maxValue, uint32_t maxSize)
{
    if (maxValue == 0 || maxSize == 0) {
        return 0;
    }

    double scaled = (static_cast<double>(value) / static_cast<double>(maxValue)) *
                    static_cast<double>(maxSize);
    return std::max<uint32_t>(1, static_cast<uint32_t>(scaled));
}
} // namespace

namespace UltraImageViewer {
namespace Core {

ImageDecoder::ImageDecoder()
{
    // Initialize WIC factory
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory2,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wicFactory_)
    );

    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create WIC factory");
    }
}

ImageDecoder::~ImageDecoder() = default;

std::unique_ptr<DecodedImage> ImageDecoder::Decode(
    const std::filesystem::path& filePath,
    DecoderFlags flags)
{
    if (!IsSupportedFormat(filePath)) {
        return nullptr;
    }

    // Check if we should use memory mapping for large files
    if (HasFlag(flags, DecoderFlags::MemoryMapped)) {
        std::error_code ec;
        uintmax_t fileSize = std::filesystem::file_size(filePath, ec);
        if (!ec && fileSize > 50 * 1024 * 1024) { // 50MB threshold
            if (auto mapped = DecodeMemoryMapped(filePath, flags)) {
                return mapped;
            }
        }
    }

    return DecodeWithWIC(filePath, flags);
}

void ImageDecoder::DecodeAsync(
    const std::filesystem::path& filePath,
    std::function<void(std::unique_ptr<DecodedImage>)> callback,
    DecoderFlags flags)
{
    if (!callback) {
        return;
    }

    // Keep async decode independent from this object's lifetime.
    std::thread([filePath, callback = std::move(callback), flags]() mutable {
        std::unique_ptr<DecodedImage> image;
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        bool shouldUninitializeCom = SUCCEEDED(hr);

        if (SUCCEEDED(hr)) {
            try {
                ImageDecoder decoder;
                image = decoder.Decode(filePath, flags);
            } catch (...) {
                image.reset();
            }
        }

        if (shouldUninitializeCom) {
            CoUninitialize();
        }

        callback(std::move(image));
    }).detach();
}

std::optional<ImageInfo> ImageDecoder::GetImageInfo(const std::filesystem::path& filePath)
{
    if (!wicFactory_) {
        return std::nullopt;
    }

    std::error_code ec;
    if (!std::filesystem::exists(filePath, ec) || ec) {
        return std::nullopt;
    }

    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = wicFactory_->CreateDecoderFromFilename(
        filePath.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand,
        &decoder
    );

    if (FAILED(hr) || !decoder) {
        return std::nullopt;
    }

    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr) || !frame) {
        return std::nullopt;
    }

    ImageInfo info = {};
    hr = frame->GetSize(&info.width, &info.height);
    if (FAILED(hr)) {
        return std::nullopt;
    }

    hr = frame->GetPixelFormat(&info.pixelFormat);
    if (FAILED(hr)) {
        return std::nullopt;
    }

    // Get bits per pixel
    Microsoft::WRL::ComPtr<IWICComponentInfo> componentInfo;
    hr = wicFactory_->CreateComponentInfo(info.pixelFormat, &componentInfo);
    if (FAILED(hr) || !componentInfo) {
        return info;
    }

    Microsoft::WRL::ComPtr<IWICPixelFormatInfo2> formatInfo;
    hr = componentInfo->QueryInterface(IID_PPV_ARGS(&formatInfo));
    if (FAILED(hr) || !formatInfo) {
        return info;
    }

    hr = formatInfo->GetBitsPerPixel(&info.bitsPerPixel);
    if (FAILED(hr)) {
        info.bitsPerPixel = 0;
    }

    return info;
}

std::unique_ptr<DecodedImage> ImageDecoder::GenerateThumbnail(
    const std::filesystem::path& filePath,
    uint32_t maxSize)
{
    if (!wicFactory_ || maxSize == 0) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = wicFactory_->CreateDecoderFromFilename(
        filePath.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand,
        &decoder
    );

    if (FAILED(hr) || !decoder) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr) || !frame) {
        return nullptr;
    }

    // Get original dimensions
    uint32_t width = 0;
    uint32_t height = 0;
    hr = frame->GetSize(&width, &height);
    if (FAILED(hr) || width == 0 || height == 0) {
        return nullptr;
    }

    // Calculate thumbnail size maintaining aspect ratio
    uint32_t thumbWidth = 0;
    uint32_t thumbHeight = 0;
    if (width > height) {
        thumbWidth = maxSize;
        thumbHeight = ScaleToMax(height, width, maxSize);
    } else {
        thumbHeight = maxSize;
        thumbWidth = ScaleToMax(width, height, maxSize);
    }

    size_t dataSize = 0;
    UINT stride = 0;
    UINT bufferSize = 0;
    if (!CalculateBgraLayout(thumbWidth, thumbHeight, dataSize, stride, bufferSize)) {
        return nullptr;
    }

    // Create thumbnail
    Microsoft::WRL::ComPtr<IWICBitmapScaler> scaler;
    hr = wicFactory_->CreateBitmapScaler(&scaler);
    if (FAILED(hr) || !scaler) {
        return nullptr;
    }

    hr = scaler->Initialize(
        frame.Get(),
        thumbWidth,
        thumbHeight,
        WICBitmapInterpolationModeFant
    );
    if (FAILED(hr)) {
        return nullptr;
    }

    // Convert to 32-bit BGRA
    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    hr = wicFactory_->CreateFormatConverter(&converter);
    if (FAILED(hr) || !converter) {
        return nullptr;
    }

    hr = converter->Initialize(
        scaler.Get(),
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );
    if (FAILED(hr)) {
        return nullptr;
    }

    // Allocate buffer
    std::unique_ptr<DecodedImage> image;
    try {
        image = std::make_unique<DecodedImage>();
        image->sourcePath = filePath;
        image->info.width = thumbWidth;
        image->info.height = thumbHeight;
        image->info.pixelFormat = GUID_WICPixelFormat32bppPBGRA;
        image->info.bitsPerPixel = 32;
        image->info.dataSize = dataSize;
        image->data = std::make_unique<uint8_t[]>(image->info.dataSize);
    } catch (const std::bad_alloc&) {
        return nullptr;
    }

    // Copy pixels
    hr = converter->CopyPixels(
        nullptr,
        stride,
        bufferSize,
        image->data.get()
    );

    if (FAILED(hr)) {
        return nullptr;
    }

    return image;
}

bool ImageDecoder::IsSupportedFormat(const std::filesystem::path& filePath)
{
    static const std::vector<std::wstring> extensions = {
        L".jpg", L".jpeg", L".png", L".bmp", L".gif",
        L".tiff", L".tif", L".webp", L".ico", L".jxr"
    };

    std::wstring ext = filePath.extension().wstring();
    Simd::ToLowerInPlace(ext);

    return std::find(extensions.begin(), extensions.end(), ext) != extensions.end();
}

std::vector<std::wstring> ImageDecoder::GetSupportedExtensions()
{
    return {
        L"*.jpg", L"*.jpeg", L"*.png", L"*.bmp", L"*.gif",
        L"*.tiff", L"*.tif", L"*.webp", L"*.ico", L"*.jxr"
    };
}

std::unique_ptr<DecodedImage> ImageDecoder::DecodeWithWIC(
    const std::filesystem::path& filePath,
    DecoderFlags flags)
{
    if (!wicFactory_) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;

    HRESULT hr = wicFactory_->CreateDecoderFromFilename(
        filePath.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand,
        &decoder
    );

    if (FAILED(hr) || !decoder) {
        return nullptr;
    }

    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr) || !frame) {
        return nullptr;
    }

    // Get image info
    std::unique_ptr<DecodedImage> image;
    try {
        image = std::make_unique<DecodedImage>();
        image->sourcePath = filePath;
    } catch (const std::bad_alloc&) {
        return nullptr;
    }

    hr = frame->GetSize(&image->info.width, &image->info.height);
    if (FAILED(hr) || image->info.width == 0 || image->info.height == 0) {
        return nullptr;
    }

    hr = frame->GetPixelFormat(&image->info.pixelFormat);
    if (FAILED(hr)) {
        return nullptr;
    }

    size_t dataSize = 0;
    UINT stride = 0;
    UINT bufferSize = 0;
    if (!CalculateBgraLayout(image->info.width, image->info.height, dataSize, stride, bufferSize)) {
        return nullptr;
    }

    // Check if format conversion is needed
    WICPixelFormatGUID targetFormat = GUID_WICPixelFormat32bppPBGRA;

    // Convert to 32-bit BGRA for GPU compatibility
    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    hr = wicFactory_->CreateFormatConverter(&converter);
    if (FAILED(hr) || !converter) {
        return nullptr;
    }

    hr = converter->Initialize(
        frame.Get(),
        targetFormat,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );

    if (FAILED(hr)) {
        return nullptr;
    }

    // Allocate buffer
    image->info.dataSize = dataSize;
    try {
        image->data = std::make_unique<uint8_t[]>(image->info.dataSize);
    } catch (const std::bad_alloc&) {
        return nullptr;
    }

    // Copy pixels
    hr = converter->CopyPixels(
        nullptr,
        stride,
        bufferSize,
        image->data.get()
    );

    if (FAILED(hr)) {
        return nullptr;
    }

    image->info.bitsPerPixel = 32;
    image->info.hasAlpha = true;
    image->info.isHDR = false;

    return image;
}

std::unique_ptr<DecodedImage> ImageDecoder::DecodeRAW(
    const std::filesystem::path& filePath,
    DecoderFlags flags)
{
    // TODO: Implement RAW decoding with libraw
    return nullptr;
}

std::unique_ptr<DecodedImage> ImageDecoder::DecodeMemoryMapped(
    const std::filesystem::path& filePath,
    DecoderFlags flags)
{
    // TODO: Implement memory-mapped file decoding
    // This requires:
    // 1. CreateFileMapping with PAGE_READONLY
    // 2. MapViewOfFile with proper alignment
    // 3. Parse image format directly from mapped memory
    // 4. Return DecodedImage with pointer to mapped view

    return nullptr;
}

} // namespace Core
} // namespace UltraImageViewer
