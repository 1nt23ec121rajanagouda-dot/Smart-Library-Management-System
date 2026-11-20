#pragma once

#include "repositories/IMemberRepository.h"
#include <string>
#include <vector>

class FileMemberRepository : public IMemberRepository {
public:
    explicit FileMemberRepository(const std::string& dataDir = "data");
    ~FileMemberRepository() override = default;

    void addMember(const Member& member) override;
    std::optional<Member> findById(int memberId) override;
    std::optional<Member> findByEmail(const std::string& email) override;
    std::vector<Member> findByName(const std::string& name) override;
    std::vector<Member> getAllMembers() override;
    std::vector<Member> getActiveMembers() override;
    void updateMember(const Member& member) override;
    void deactivateMember(int memberId) override;
    int getNextId() override;

private:
    std::string filePath_;
    std::vector<Member> members_;

    void loadFromFile();
    void saveToFile();
    static std::string toLowerStr(const std::string& s);
};
