#pragma once

#include <string>
#include <sstream>
#include <stdexcept>

class Admin {
public:
    Admin() = default;

    Admin(int userId, const std::string& username, const std::string& passwordHash)
        : userId_(userId), username_(username), passwordHash_(passwordHash), isActive_(true) {
        validate();
    }

    // ── Getters ────────────────────────────────────────────────────────────
    int getUserId()                  const { return userId_; }
    const std::string& getUsername()     const { return username_; }
    const std::string& getPasswordHash() const { return passwordHash_; }
    bool getIsActive()               const { return isActive_; }

    // ── Setters ────────────────────────────────────────────────────────────
    void setUserId(int id)                       { userId_ = id; }
    void setUsername(const std::string& u)        { username_ = u; }
    void setPasswordHash(const std::string& h)   { passwordHash_ = h; }
    void setIsActive(bool a)                     { isActive_ = a; }

    // ── Validation ─────────────────────────────────────────────────────────
    void validate() const {
        if (username_.empty()) throw std::invalid_argument("Username cannot be empty");
        if (passwordHash_.empty()) throw std::invalid_argument("Password hash cannot be empty");
    }

    // ── Serialization (pipe-delimited) ─────────────────────────────────────
    std::string serialize() const {
        std::ostringstream oss;
        oss << userId_ << "|"
            << username_ << "|"
            << passwordHash_ << "|"
            << (isActive_ ? 1 : 0);
        return oss.str();
    }

    static Admin deserialize(const std::string& line) {
        Admin a;
        std::istringstream iss(line);
        std::string token;

        auto next = [&]() -> std::string {
            std::getline(iss, token, '|');
            return token;
        };

        a.userId_       = std::stoi(next());
        a.username_     = next();
        a.passwordHash_ = next();
        a.isActive_     = (next() == "1");

        return a;
    }

private:
    int userId_ = 0;
    std::string username_;
    std::string passwordHash_;
    bool isActive_ = true;
};
