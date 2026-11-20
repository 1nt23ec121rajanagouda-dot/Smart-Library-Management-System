#include "repositories/FileMemberRepository.h"

#include <fstream>
#include <algorithm>
#include <filesystem>
#include <cctype>

FileMemberRepository::FileMemberRepository(const std::string& dataDir)
    : filePath_(dataDir + "/members.dat") {
    std::filesystem::create_directories(dataDir);
    loadFromFile();
}

// ── Private helpers ────────────────────────────────────────────────────────

void FileMemberRepository::loadFromFile() {
    members_.clear();
    std::ifstream in(filePath_);
    if (!in.is_open()) return;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            members_.push_back(Member::deserialize(line));
        }
    }
}

void FileMemberRepository::saveToFile() {
    std::ofstream out(filePath_, std::ios::trunc);
    for (const auto& member : members_) {
        out << member.serialize() << "\n";
    }
}

std::string FileMemberRepository::toLowerStr(const std::string& s) {
    std::string lower;
    lower.reserve(s.size());
    for (char c : s) lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return lower;
}

// ── Interface implementation ───────────────────────────────────────────────

void FileMemberRepository::addMember(const Member& member) {
    members_.push_back(member);
    saveToFile();
}

std::optional<Member> FileMemberRepository::findById(int memberId) {
    for (const auto& m : members_) {
        if (m.getMemberId() == memberId) return m;
    }
    return std::nullopt;
}

std::optional<Member> FileMemberRepository::findByEmail(const std::string& email) {
    std::string lowerEmail = toLowerStr(email);
    for (const auto& m : members_) {
        if (toLowerStr(m.getEmail()) == lowerEmail) {
            return m;
        }
    }
    return std::nullopt;
}

std::vector<Member> FileMemberRepository::findByName(const std::string& name) {
    std::string lowerName = toLowerStr(name);
    std::vector<Member> result;
    for (const auto& m : members_) {
        if (toLowerStr(m.getName()).find(lowerName) != std::string::npos) {
            result.push_back(m);
        }
    }
    return result;
}

std::vector<Member> FileMemberRepository::getAllMembers() {
    return members_;
}

std::vector<Member> FileMemberRepository::getActiveMembers() {
    std::vector<Member> result;
    for (const auto& m : members_) {
        if (m.getIsActive()) result.push_back(m);
    }
    return result;
}

void FileMemberRepository::updateMember(const Member& member) {
    for (auto& m : members_) {
        if (m.getMemberId() == member.getMemberId()) {
            m = member;
            saveToFile();
            return;
        }
    }
}

void FileMemberRepository::deactivateMember(int memberId) {
    for (auto& m : members_) {
        if (m.getMemberId() == memberId) {
            m.setIsActive(false);
            saveToFile();
            return;
        }
    }
}

int FileMemberRepository::getNextId() {
    int maxId = 0;
    for (const auto& m : members_) {
        if (m.getMemberId() > maxId) maxId = m.getMemberId();
    }
    return maxId + 1;
}
