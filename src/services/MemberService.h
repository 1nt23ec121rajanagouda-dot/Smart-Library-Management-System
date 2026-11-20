#pragma once

#include <string>
#include <vector>
#include <utility>
#include <optional>
#include "models/Member.h"
#include "repositories/IMemberRepository.h"
#include "repositories/ILoanRepository.h"

class MemberService {
public:
    MemberService(IMemberRepository& memberRepo, ILoanRepository& loanRepo);

    std::pair<bool, std::string> registerMember(const std::string& name,
                                                const std::string& email,
                                                const std::string& phone);

    std::vector<Member> getAllMembers();
    std::vector<Member> getActiveMembers();
    std::vector<Member> searchMembers(const std::string& query);

    std::pair<bool, std::string> updateMember(int id,
                                              const std::string& name,
                                              const std::string& email,
                                              const std::string& phone);

    std::pair<bool, std::string> deactivateMember(int id);

private:
    IMemberRepository& memberRepo_;
    ILoanRepository& loanRepo_;
};
