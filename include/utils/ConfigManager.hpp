#pragma once

#include <string>
#include <filesystem>
#include <thread>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace UltraImageViewer {
namespace Utils {

class ConfigManager {
public:
    struct WindowSettings {
        uint32_t width = 1280;
        uint32_t height = 720;
        uint32_t maxWidth = 7680;
        uint32_t maxHeight = 4320;
    };

    struct CacheSettings {
        size_t maxSizeMB = 512;
    };

    struct PerformanceSettings {
        uint32_t maxThreads = std::thread::hardware_concurrency();
        bool enableSIMD = true;
    };

    struct UISettings {
        std::string theme = "dark";
        bool showThumbnails = true;
        bool enableGestures = true;
    };

    struct Config {
        WindowSettings window;
        CacheSettings cache;
        PerformanceSettings performance;
        UISettings ui;
    };

    ConfigManager();
    ~ConfigManager();

    static ConfigManager& GetInstance();

    bool Load(const std::filesystem::path& path = {});
    bool Save();
    bool ValidateConfig() const;
    void ResetToDefaults();

    bool GetBool(const std::string& key, bool defaultValue = false) const;
    int GetInt(const std::string& key, int defaultValue = 0) const;
    float GetFloat(const std::string& key, float defaultValue = 0.0f) const;
    std::string GetString(const std::string& key, const std::string& defaultValue = "") const;

    void SetBool(const std::string& key, bool value);
    void SetInt(const std::string& key, int value);
    void SetFloat(const std::string& key, float value);
    void SetString(const std::string& key, const std::string& value);

    const Config& GetConfig() const;

private:
    std::filesystem::path configFilePath_;
    Config config_;
};

// JSON (de)serialization for config structures
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConfigManager::WindowSettings, width, height, maxWidth, maxHeight)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConfigManager::CacheSettings, maxSizeMB)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConfigManager::PerformanceSettings, maxThreads, enableSIMD)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConfigManager::UISettings, theme, showThumbnails, enableGestures)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConfigManager::Config, window, cache, performance, ui)

} // namespace Utils
} // namespace UltraImageViewer
