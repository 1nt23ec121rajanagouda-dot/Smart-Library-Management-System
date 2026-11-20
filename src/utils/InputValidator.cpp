#include "utils/InputValidator.h"

#include <iostream>
#include <limits>
#include <regex>
#include <algorithm>

namespace InputValidator {

bool isNonEmpty(const std::string& s) {
    return !s.empty() &&
           s.find_first_not_of(" \t\n\r") != std::string::npos;
}

bool isValidInteger(const std::string& s) {
    if (s.empty()) return false;
    size_t start = (s[0] == '-' || s[0] == '+') ? 1 : 0;
    if (start >= s.size()) return false;
    return std::all_of(s.begin() + static_cast<long>(start), s.end(), ::isdigit);
}

bool isValidEmail(const std::string& email) {
    // Basic email regex: something@something.something
    static const std::regex pattern(R"([a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email, pattern);
}

bool isValidPhone(const std::string& phone) {
    // Accept digits, spaces, dashes, parens, plus sign; at least 7 digits
    static const std::regex pattern(R"([\d\s\-\+\(\)]{7,20})");
    if (!std::regex_match(phone, pattern)) return false;
    // Count actual digits
    int digitCount = 0;
    for (char c : phone) {
        if (std::isdigit(static_cast<unsigned char>(c))) digitCount++;
    }
    return digitCount >= 7;
}

bool isValidISBN(const std::string& isbn) {
    // Accept ISBN-10 (10 digits, last may be X) or ISBN-13 (13 digits)
    // Strip hyphens/spaces first
    std::string clean;
    for (char c : isbn) {
        if (c != '-' && c != ' ') clean += c;
    }
    if (clean.size() == 10) {
        for (size_t i = 0; i < 9; ++i) {
            if (!std::isdigit(static_cast<unsigned char>(clean[i]))) return false;
        }
        char last = clean[9];
        return std::isdigit(static_cast<unsigned char>(last)) || last == 'X' || last == 'x';
    }
    if (clean.size() == 13) {
        return std::all_of(clean.begin(), clean.end(),
                           [](char c) { return std::isdigit(static_cast<unsigned char>(c)); });
    }
    return false;
}

int getValidatedInt(const std::string& prompt, int minVal, int maxVal) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (value >= minVal && value <= maxVal) {
                return value;
            }
            std::cout << "  Value must be between " << minVal << " and " << maxVal << ".\n";
        } else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  Invalid input. Please enter a number.\n";
        }
    }
}

std::string getValidatedString(const std::string& prompt) {
    std::string value;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, value);
        if (isNonEmpty(value)) {
            return value;
        }
        std::cout << "  Input cannot be empty.\n";
    }
}

bool getConfirmation(const std::string& prompt) {
    std::string input;
    while (true) {
        std::cout << prompt << " (y/n): ";
        std::getline(std::cin, input);
        if (!input.empty()) {
            char c = static_cast<char>(std::tolower(static_cast<unsigned char>(input[0])));
            if (c == 'y') return true;
            if (c == 'n') return false;
        }
        std::cout << "  Please enter 'y' or 'n'.\n";
    }
}

} // namespace InputValidator
