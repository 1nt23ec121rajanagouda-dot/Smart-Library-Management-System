#include "utils/DateUtils.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <regex>

namespace DateUtils {

TimePoint getCurrentDate() {
    return std::chrono::system_clock::now();
}

TimePoint addDays(const TimePoint& date, int n) {
    return date + std::chrono::hours(24 * n);
}

int daysBetween(const TimePoint& date1, const TimePoint& date2) {
    auto diff = date2 - date1;
    return static_cast<int>(
        std::chrono::duration_cast<std::chrono::hours>(diff).count() / 24);
}

TimePoint parseDate(const std::string& dateStr) {
    std::tm tm = {};
    std::istringstream ss(dateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) {
        throw std::invalid_argument("Invalid date format: " + dateStr + " (expected YYYY-MM-DD)");
    }
    tm.tm_hour = 0;
    tm.tm_min  = 0;
    tm.tm_sec  = 0;
    tm.tm_isdst = -1;
    std::time_t t = std::mktime(&tm);
    if (t == -1) {
        throw std::invalid_argument("Cannot convert date: " + dateStr);
    }
    return std::chrono::system_clock::from_time_t(t);
}

std::string formatDate(const TimePoint& date) {
    std::time_t t = std::chrono::system_clock::to_time_t(date);
    std::tm tm;
    localtime_r(&t, &tm);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

bool isValidDate(const std::string& dateStr) {
    // Quick format check
    static const std::regex pattern(R"(\d{4}-\d{2}-\d{2})");
    if (!std::regex_match(dateStr, pattern)) return false;

    // Try to actually parse it
    try {
        parseDate(dateStr);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace DateUtils
