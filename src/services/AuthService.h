#pragma once

#include <string>
#include <utility>
#include <optional>
#include "repositories/IAdminRepository.h"

class AuthService {
public:
    explicit AuthService(IAdminRepository& adminRepo);

    bool login(const std::string& username, const std::string& password);
    std::pair<bool, std::string> createAdmin(const std::string& username, const std::string& password);
    bool isFirstRun() const;
    int getCurrentAdminId() const;

private:
    IAdminRepository& adminRepo_;
    int currentAdminId_ = -1;
};
