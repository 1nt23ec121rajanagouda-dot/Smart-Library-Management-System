#include "services/AuthService.h"
#include "utils/PasswordUtils.h"

AuthService::AuthService(IAdminRepository& adminRepo)
    : adminRepo_(adminRepo) {}

bool AuthService::login(const std::string& username, const std::string& password) {
    auto admin = adminRepo_.findByUsername(username);
    if (!admin.has_value()) {
        return false;
    }

    if (!admin->getIsActive()) {
        return false;
    }

    std::string hashedInput = PasswordUtils::hashPassword(password);
    if (!PasswordUtils::verifyPassword(password, admin->getPasswordHash())) {
        return false;
    }

    currentAdminId_ = admin->getUserId();
    return true;
}

std::pair<bool, std::string> AuthService::createAdmin(const std::string& username, const std::string& password) {
    if (username.empty()) {
        return {false, "Username cannot be empty."};
    }

    if (password.length() < 6) {
        return {false, "Password must be at least 6 characters long."};
    }

    auto existing = adminRepo_.findByUsername(username);
    if (existing.has_value()) {
        return {false, "An admin with this username already exists."};
    }

    std::string passwordHash = PasswordUtils::hashPassword(password);

    Admin admin;
    admin.setUserId(adminRepo_.getNextId());
    admin.setUsername(username);
    admin.setPasswordHash(passwordHash);
    admin.setIsActive(true);

    adminRepo_.addAdmin(admin);

    return {true, "Admin account created successfully."};
}

bool AuthService::isFirstRun() const {
    return adminRepo_.getAllAdmins().empty();
}

int AuthService::getCurrentAdminId() const {
    return currentAdminId_;
}
