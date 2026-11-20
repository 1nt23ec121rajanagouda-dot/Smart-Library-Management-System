#include "services/MemberService.h"
#include "utils/DateUtils.h"

MemberService::MemberService(IMemberRepository& memberRepo, ILoanRepository& loanRepo)
    : memberRepo_(memberRepo), loanRepo_(loanRepo) {}

std::pair<bool, std::string> MemberService::registerMember(const std::string& name,
                                                            const std::string& email,
                                                            const std::string& phone) {
    if (name.empty()) {
        return {false, "Name cannot be empty."};
    }
    if (email.empty()) {
        return {false, "Email cannot be empty."};
    }
    if (phone.empty()) {
        return {false, "Phone number cannot be empty."};
    }

    // Check email uniqueness
    auto existingMember = memberRepo_.findByEmail(email);
    if (existingMember.has_value()) {
        return {false, "A member with this email already exists."};
    }

    Member member;
    member.setMemberId(memberRepo_.getNextId());
    member.setName(name);
    member.setEmail(email);
    member.setPhone(phone);
    member.setRegisteredAt(DateUtils::getCurrentDate());
    member.setIsActive(true);

    memberRepo_.addMember(member);

    return {true, "Member registered successfully with ID: " + std::to_string(member.getMemberId()) + "."};
}

std::vector<Member> MemberService::getAllMembers() {
    return memberRepo_.getAllMembers();
}

std::vector<Member> MemberService::getActiveMembers() {
    return memberRepo_.getActiveMembers();
}

std::vector<Member> MemberService::searchMembers(const std::string& query) {
    std::vector<Member> results;

    // Try searching by name
    auto byName = memberRepo_.findByName(query);
    results.insert(results.end(), byName.begin(), byName.end());

    // Try searching by ID (if query is a valid integer)
    try {
        int id = std::stoi(query);
        auto byId = memberRepo_.findById(id);
        if (byId.has_value()) {
            // Avoid duplicates — check if already in results
            bool alreadyPresent = false;
            for (const auto& m : results) {
                if (m.getMemberId() == byId->getMemberId()) {
                    alreadyPresent = true;
                    break;
                }
            }
            if (!alreadyPresent) {
                results.push_back(byId.value());
            }
        }
    } catch (...) {
        // Not a valid integer, skip ID search
    }

    // Try searching by email
    auto byEmail = memberRepo_.findByEmail(query);
    if (byEmail.has_value()) {
        bool alreadyPresent = false;
        for (const auto& m : results) {
            if (m.getMemberId() == byEmail->getMemberId()) {
                alreadyPresent = true;
                break;
            }
        }
        if (!alreadyPresent) {
            results.push_back(byEmail.value());
        }
    }

    return results;
}

std::pair<bool, std::string> MemberService::updateMember(int id,
                                                          const std::string& name,
                                                          const std::string& email,
                                                          const std::string& phone) {
    auto memberOpt = memberRepo_.findById(id);
    if (!memberOpt.has_value()) {
        return {false, "Member not found."};
    }

    if (name.empty()) {
        return {false, "Name cannot be empty."};
    }
    if (email.empty()) {
        return {false, "Email cannot be empty."};
    }
    if (phone.empty()) {
        return {false, "Phone number cannot be empty."};
    }

    // Check email uniqueness if changed
    auto existingMember = memberRepo_.findByEmail(email);
    if (existingMember.has_value() && existingMember->getMemberId() != id) {
        return {false, "Another member with this email already exists."};
    }

    Member member = memberOpt.value();
    member.setName(name);
    member.setEmail(email);
    member.setPhone(phone);

    memberRepo_.updateMember(member);

    return {true, "Member updated successfully."};
}

std::pair<bool, std::string> MemberService::deactivateMember(int id) {
    auto memberOpt = memberRepo_.findById(id);
    if (!memberOpt.has_value()) {
        return {false, "Member not found."};
    }

    // Check for active loans
    auto activeLoans = loanRepo_.findActiveByMember(id);
    if (!activeLoans.empty()) {
        return {false, "Cannot deactivate member. They have " +
                std::to_string(activeLoans.size()) + " active loan(s)."};
    }

    memberRepo_.deactivateMember(id);

    return {true, "Member deactivated successfully."};
}
