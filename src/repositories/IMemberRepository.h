#pragma once

#include "models/Member.h"
#include <vector>
#include <optional>
#include <string>

class IMemberRepository {
public:
    virtual ~IMemberRepository() = default;

    virtual void addMember(const Member& member) = 0;
    virtual std::optional<Member> findById(int memberId) = 0;
    virtual std::optional<Member> findByEmail(const std::string& email) = 0;
    virtual std::vector<Member> findByName(const std::string& name) = 0;
    virtual std::vector<Member> getAllMembers() = 0;
    virtual std::vector<Member> getActiveMembers() = 0;
    virtual void updateMember(const Member& member) = 0;
    virtual void deactivateMember(int memberId) = 0;
    virtual int getNextId() = 0;
};
