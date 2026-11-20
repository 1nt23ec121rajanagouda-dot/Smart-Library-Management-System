#pragma once

#include <string>

namespace InputValidator {

// Validation checks (return true if valid)
bool isNonEmpty(const std::string& s);
bool isValidInteger(const std::string& s);
bool isValidEmail(const std::string& email);
bool isValidPhone(const std::string& phone);
bool isValidISBN(const std::string& isbn);

// Interactive input from cin with error recovery
int getValidatedInt(const std::string& prompt, int minVal = 0, int maxVal = 999999);
std::string getValidatedString(const std::string& prompt);
bool getConfirmation(const std::string& prompt);

} // namespace InputValidator
