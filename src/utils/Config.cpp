#include "utils/Config.h"

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <iostream>

Config& Config::instance() {
    static Config cfg;
    return cfg;
}

void Config::load(const std::string& envFilePath) {
    if (loaded_) return;

    // Set defaults
    values_["DB_HOST"]          = "localhost";
    values_["DB_PORT"]          = "3306";
    values_["DB_NAME"]          = "library_management";
    values_["DB_USER"]          = "library_user";
    values_["DB_PASSWORD"]      = "";
    values_["LOAN_PERIOD_DAYS"] = "14";
    values_["FINE_RATE_PER_DAY"] = "5";
    values_["MAX_ACTIVE_LOANS"] = "3";
    values_["STORAGE_MODE"]     = "file";

    // Load .env file (overrides defaults)
    loadEnvFile(envFilePath);

    // Environment variables override everything
    loadEnvironmentVariables();

    loaded_ = true;
}

void Config::loadEnvFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        auto eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key   = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);

        // Trim whitespace
        auto trim = [](std::string& s) {
            s.erase(0, s.find_first_not_of(" \t\r\n"));
            s.erase(s.find_last_not_of(" \t\r\n") + 1);
        };
        trim(key);
        trim(value);

        // Remove surrounding quotes if present
        if (value.size() >= 2 &&
            ((value.front() == '"' && value.back() == '"') ||
             (value.front() == '\'' && value.back() == '\''))) {
            value = value.substr(1, value.size() - 2);
        }

        if (!key.empty()) {
            values_[key] = value;
        }
    }
}

void Config::loadEnvironmentVariables() {
    static const char* keys[] = {
        "DB_HOST", "DB_PORT", "DB_NAME", "DB_USER", "DB_PASSWORD",
        "LOAN_PERIOD_DAYS", "FINE_RATE_PER_DAY", "MAX_ACTIVE_LOANS",
        "STORAGE_MODE"
    };

    for (const char* key : keys) {
        const char* val = std::getenv(key);
        if (val != nullptr) {
            values_[key] = val;
        }
    }
}

// ── Typed getters ──────────────────────────────────────────────────────────

std::string Config::get(const std::string& key, const std::string& defaultValue) const {
    auto it = values_.find(key);
    return (it != values_.end()) ? it->second : defaultValue;
}

int Config::getInt(const std::string& key, int defaultValue) const {
    auto it = values_.find(key);
    if (it == values_.end()) return defaultValue;
    try {
        return std::stoi(it->second);
    } catch (...) {
        return defaultValue;
    }
}

std::string Config::getDbHost()     const { return get("DB_HOST", "localhost"); }
int         Config::getDbPort()     const { return getInt("DB_PORT", 3306); }
std::string Config::getDbName()     const { return get("DB_NAME", "library_management"); }
std::string Config::getDbUser()     const { return get("DB_USER", "library_user"); }
std::string Config::getDbPassword() const { return get("DB_PASSWORD", ""); }
int         Config::getLoanPeriodDays()  const { return getInt("LOAN_PERIOD_DAYS", 14); }
int         Config::getFineRatePerDay()  const { return getInt("FINE_RATE_PER_DAY", 5); }
int         Config::getMaxActiveLoans()  const { return getInt("MAX_ACTIVE_LOANS", 3); }
std::string Config::getStorageMode()     const { return get("STORAGE_MODE", "file"); }
