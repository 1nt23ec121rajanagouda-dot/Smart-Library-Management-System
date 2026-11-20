#pragma once

#include <string>
#include <sstream>
#include <stdexcept>
#include <chrono>

class Member {
public:
    Member() = default;

    Member(int id, const std::string& name, const std::string& email,
           const std::string& phone)
        : memberId_(id), name_(name), email_(email), phone_(phone), isActive_(true) {
        registeredAt_ = std::chrono::system_clock::now();
        validate();
    }

    // ── Getters ────────────────────────────────────────────────────────────
    int getMemberId()            const { return memberId_; }
    const std::string& getName()     const { return name_; }
    const std::string& getEmail()    const { return email_; }
    const std::string& getPhone()    const { return phone_; }
    std::chrono::system_clock::time_point getRegisteredAt() const { return registeredAt_; }
    bool getIsActive()           const { return isActive_; }

    // ── Setters ────────────────────────────────────────────────────────────
    void setMemberId(int id)              { memberId_ = id; }
    void setName(const std::string& n)    { name_ = n; }
    void setEmail(const std::string& e)   { email_ = e; }
    void setPhone(const std::string& p)   { phone_ = p; }
    void setRegisteredAt(std::chrono::system_clock::time_point t) { registeredAt_ = t; }
    void setIsActive(bool a)              { isActive_ = a; }

    // ── Validation ─────────────────────────────────────────────────────────
    void validate() const {
        if (name_.empty()) throw std::invalid_argument("Member name cannot be empty");
        if (email_.empty()) throw std::invalid_argument("Member email cannot be empty");
        if (phone_.empty()) throw std::invalid_argument("Member phone cannot be empty");
    }

    // ── Serialization (pipe-delimited) ─────────────────────────────────────
    std::string serialize() const {
        std::ostringstream oss;
        oss << memberId_ << "|"
            << name_ << "|"
            << email_ << "|"
            << phone_ << "|"
            << std::chrono::system_clock::to_time_t(registeredAt_) << "|"
            << (isActive_ ? 1 : 0);
        return oss.str();
    }

    static Member deserialize(const std::string& line) {
        Member m;
        std::istringstream iss(line);
        std::string token;

        auto next = [&]() -> std::string {
            std::getline(iss, token, '|');
            return token;
        };

        m.memberId_     = std::stoi(next());
        m.name_         = next();
        m.email_        = next();
        m.phone_        = next();
        m.registeredAt_ = std::chrono::system_clock::from_time_t(std::stoll(next()));
        m.isActive_     = (next() == "1");

        return m;
    }

private:
    int memberId_ = 0;
    std::string name_;
    std::string email_;
    std::string phone_;
    std::chrono::system_clock::time_point registeredAt_;
    bool isActive_ = true;
};
