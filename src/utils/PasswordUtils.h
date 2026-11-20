#pragma once

#include <string>

namespace PasswordUtils {

// Hash a plaintext password using SHA-256 (CommonCrypto), returns hex digest
std::string hashPassword(const std::string& password);

// Verify a plaintext password against a stored hex hash
bool verifyPassword(const std::string& password, const std::string& hash);

} // namespace PasswordUtils
