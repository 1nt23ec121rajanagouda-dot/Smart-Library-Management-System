#pragma once

#include "models/Admin.h"
#include <vector>
#include <optional>
#include <string>

class IAdminRepository {
public:
    virtual ~IAdminRepository() = default;

    virtual void addAdmin(const Admin& admin) = 0;
    virtual std::optional<Admin> findByUsername(const std::string& username) = 0;
    virtual std::vector<Admin> getAllAdmins() = 0;
    virtual int getNextId() = 0;
};
