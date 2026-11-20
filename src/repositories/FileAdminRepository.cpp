#include "repositories/FileAdminRepository.h"

#include <fstream>
#include <algorithm>
#include <filesystem>
#include <cctype>

FileAdminRepository::FileAdminRepository(const std::string& dataDir)
    : filePath_(dataDir + "/admins.dat") {
    std::filesystem::create_directories(dataDir);
    loadFromFile();
}

// ── Private helpers ────────────────────────────────────────────────────────

void FileAdminRepository::loadFromFile() {
    admins_.clear();
    std::ifstream in(filePath_);
    if (!in.is_open()) return;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            admins_.push_back(Admin::deserialize(line));
        }
    }
}

void FileAdminRepository::saveToFile() {
    std::ofstream out(filePath_, std::ios::trunc);
    for (const auto& admin : admins_) {
        out << admin.serialize() << "\n";
    }
}

std::string FileAdminRepository::toLowerStr(const std::string& s) {
    std::string lower;
    lower.reserve(s.size());
    for (char c : s) lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return lower;
}

// ── Interface implementation ───────────────────────────────────────────────

void FileAdminRepository::addAdmin(const Admin& admin) {
    admins_.push_back(admin);
    saveToFile();
}

std::optional<Admin> FileAdminRepository::findByUsername(const std::string& username) {
    std::string lowerUsername = toLowerStr(username);
    for (const auto& a : admins_) {
        if (toLowerStr(a.getUsername()) == lowerUsername) {
            return a;
        }
    }
    return std::nullopt;
}

std::vector<Admin> FileAdminRepository::getAllAdmins() {
    return admins_;
}

int FileAdminRepository::getNextId() {
    int maxId = 0;
    for (const auto& a : admins_) {
        if (a.getUserId() > maxId) maxId = a.getUserId();
    }
    return maxId + 1;
}
