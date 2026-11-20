#include "utils/PasswordUtils.h"

#include <CommonCrypto/CommonDigest.h>
#include <iomanip>
#include <sstream>

namespace PasswordUtils {

std::string hashPassword(const std::string& password) {
    unsigned char digest[CC_SHA256_DIGEST_LENGTH];

    CC_SHA256(password.c_str(),
              static_cast<CC_LONG>(password.size()),
              digest);

    std::ostringstream oss;
    for (int i = 0; i < CC_SHA256_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(digest[i]);
    }
    return oss.str();
}

bool verifyPassword(const std::string& password, const std::string& hash) {
    return hashPassword(password) == hash;
}

} // namespace PasswordUtils
