#pragma once

#include <string>
#include <chrono>

namespace DateUtils {

using TimePoint = std::chrono::system_clock::time_point;

// Get the current date/time
TimePoint getCurrentDate();

// Add n days to a time_point
TimePoint addDays(const TimePoint& date, int n);

// Return number of days between two dates (date2 - date1)
int daysBetween(const TimePoint& date1, const TimePoint& date2);

// Parse an ISO date string "YYYY-MM-DD" into a time_point
TimePoint parseDate(const std::string& dateStr);

// Format a time_point as "YYYY-MM-DD"
std::string formatDate(const TimePoint& date);

// Check if a string is a valid "YYYY-MM-DD" date
bool isValidDate(const std::string& dateStr);

} // namespace DateUtils
