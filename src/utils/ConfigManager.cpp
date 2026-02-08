#include "utils/ConfigManager.hpp"
#include <fstream>
#include <sstream>

namespace UltraImageViewer {
namespace Utils {

ConfigManager::ConfigManager()
    : configFilePath_(L"config.json")
{
}

ConfigManager::~ConfigManager()
{
    Save();
}

ConfigManager& ConfigManager::GetInstance()
{
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::Load(const std::filesystem::path& path)
{
    if (!path.empty()) {
        configFilePath_ = path;
    }

    std::ifstream file(configFilePath_);
    if (!file.is_open()) {
        // Use default config
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;
        config_ = j.get<Config>();

        // Validate config
        return ValidateConfig();
    } catch (const std::exception& e) {
        return false;
    }
}

bool ConfigManager::Save()
{
    try {
        nlohmann::json j = config_;
        std::ofstream file(configFilePath_);
        file << j.dump(4);  // Pretty print with 4 spaces
        return file.good();
    } catch (const std::exception& e) {
        return false;
    }
}

bool ConfigManager::ValidateConfig() const
{
    // Validate window settings
    if (config_.window.maxWidth < 640 || config_.window.maxWidth > 7680) {
        return false;
    }
    if (config_.window.maxHeight < 480 || config_.window.maxHeight > 4320) {
        return false;
    }

    // Validate cache settings
    if (config_.cache.maxSizeMB == 0 || config_.cache.maxSizeMB > 8192) {
        return false;
    }

    // Validate performance settings
    if (config_.performance.maxThreads == 0 || config_.performance.maxThreads > 64) {
        return false;
    }

    return true;
}

void ConfigManager::ResetToDefaults()
{
    config_ = Config();
    config_.window.width = 1280;
    config_.window.height = 720;
    config_.cache.maxSizeMB = 512;
    config_.performance.maxThreads = std::thread::hardware_concurrency();
    config_.performance.enableSIMD = true;
    config_.ui.theme = "dark";
    config_.ui.showThumbnails = true;
    config_.ui.enableGestures = true;
}

bool ConfigManager::GetBool(const std::string& key, bool defaultValue) const
{
    try {
        nlohmann::json j = config_;
        std::vector<std::string> parts;
        std::stringstream ss(key);
        std::string part;

        while (std::getline(ss, part, '.')) {
            parts.push_back(part);
        }

        for (size_t i = 0; i < parts.size(); ++i) {
            if (j.contains(parts[i])) {
                if (i == parts.size() - 1) {
                    return j[parts[i]].get<bool>();
                }
                j = j[parts[i]];
            } else {
                return defaultValue;
            }
        }
    } catch (...) {
        // Fall through to default
    }

    return defaultValue;
}

int ConfigManager::GetInt(const std::string& key, int defaultValue) const
{
    try {
        nlohmann::json j = config_;
        std::vector<std::string> parts;
        std::stringstream ss(key);
        std::string part;

        while (std::getline(ss, part, '.')) {
            parts.push_back(part);
        }

        for (size_t i = 0; i < parts.size(); ++i) {
            if (j.contains(parts[i])) {
                if (i == parts.size() - 1) {
                    return j[parts[i]].get<int>();
                }
                j = j[parts[i]];
            } else {
                return defaultValue;
            }
        }
    } catch (...) {
        // Fall through to default
    }

    return defaultValue;
}

float ConfigManager::GetFloat(const std::string& key, float defaultValue) const
{
    try {
        nlohmann::json j = config_;
        std::vector<std::string> parts;
        std::stringstream ss(key);
        std::string part;

        while (std::getline(ss, part, '.')) {
            parts.push_back(part);
        }

        for (size_t i = 0; i < parts.size(); ++i) {
            if (j.contains(parts[i])) {
                if (i == parts.size() - 1) {
                    return j[parts[i]].get<float>();
                }
                j = j[parts[i]];
            } else {
                return defaultValue;
            }
        }
    } catch (...) {
        // Fall through to default
    }

    return defaultValue;
}

std::string ConfigManager::GetString(const std::string& key, const std::string& defaultValue) const
{
    try {
        nlohmann::json j = config_;
        std::vector<std::string> parts;
        std::stringstream ss(key);
        std::string part;

        while (std::getline(ss, part, '.')) {
            parts.push_back(part);
        }

        for (size_t i = 0; i < parts.size(); ++i) {
            if (j.contains(parts[i])) {
                if (i == parts.size() - 1) {
                    return j[parts[i]].get<std::string>();
                }
                j = j[parts[i]];
            } else {
                return defaultValue;
            }
        }
    } catch (...) {
        // Fall through to default
    }

    return defaultValue;
}

void ConfigManager::SetBool(const std::string& key, bool value)
{
    nlohmann::json j = config_;
    j[key] = value;
    config_ = j.get<Config>();
}

void ConfigManager::SetInt(const std::string& key, int value)
{
    nlohmann::json j = config_;
    j[key] = value;
    config_ = j.get<Config>();
}

void ConfigManager::SetFloat(const std::string& key, float value)
{
    nlohmann::json j = config_;
    j[key] = value;
    config_ = j.get<Config>();
}

void ConfigManager::SetString(const std::string& key, const std::string& value)
{
    nlohmann::json j = config_;
    j[key] = value;
    config_ = j.get<Config>();
}

const ConfigManager::Config& ConfigManager::GetConfig() const
{
    return config_;
}

} // namespace Utils
} // namespace UltraImageViewer
