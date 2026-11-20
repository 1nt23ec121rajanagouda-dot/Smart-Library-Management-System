#pragma once

#include <string>
#include <unordered_map>

class Config {
public:
    // Singleton access
    static Config& instance();

    // Load from .env file (if exists) and environment variables
    // Environment variables override .env file values
    void load(const std::string& envFilePath = ".env");

    // Getters for specific keys
    std::string getDbHost()     const;
    int         getDbPort()     const;
    std::string getDbName()     const;
    std::string getDbUser()     const;
    std::string getDbPassword() const;

    int    getLoanPeriodDays()  const;
    int    getFineRatePerDay()  const;
    int    getMaxActiveLoans()  const;
    std::string getStorageMode() const;

    // Generic get
    std::string get(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;

private:
    Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    void loadEnvFile(const std::string& path);
    void loadEnvironmentVariables();

    std::unordered_map<std::string, std::string> values_;
    bool loaded_ = false;
};
