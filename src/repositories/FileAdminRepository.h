#pragma once

#include "repositories/IAdminRepository.h"
#include <string>
#include <vector>

class FileAdminRepository : public IAdminRepository {
public:
    explicit FileAdminRepository(const std::string& dataDir = "data");
    ~FileAdminRepository() override = default;

    void addAdmin(const Admin& admin) override;
    std::optional<Admin> findByUsername(const std::string& username) override;
    std::vector<Admin> getAllAdmins() override;
    int getNextId() override;

private:
    std::string filePath_;
    std::vector<Admin> admins_;

    void loadFromFile();
    void saveToFile();
    static std::string toLowerStr(const std::string& s);
};
