#include "repositories/FileLoanRepository.h"

#include <fstream>
#include <algorithm>
#include <filesystem>
#include <unordered_map>

FileLoanRepository::FileLoanRepository(const std::string& dataDir)
    : filePath_(dataDir + "/loans.dat") {
    std::filesystem::create_directories(dataDir);
    loadFromFile();
}

// ── Private helpers ────────────────────────────────────────────────────────

void FileLoanRepository::loadFromFile() {
    loans_.clear();
    std::ifstream in(filePath_);
    if (!in.is_open()) return;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            loans_.push_back(Loan::deserialize(line));
        }
    }
}

void FileLoanRepository::saveToFile() {
    std::ofstream out(filePath_, std::ios::trunc);
    for (const auto& loan : loans_) {
        out << loan.serialize() << "\n";
    }
}

// ── Interface implementation ───────────────────────────────────────────────

void FileLoanRepository::createLoan(const Loan& loan) {
    loans_.push_back(loan);
    saveToFile();
}

std::optional<Loan> FileLoanRepository::findById(int loanId) {
    for (const auto& l : loans_) {
        if (l.getLoanId() == loanId) return l;
    }
    return std::nullopt;
}

std::vector<Loan> FileLoanRepository::findActiveByMember(int memberId) {
    std::vector<Loan> result;
    for (const auto& l : loans_) {
        if (l.getMemberId() == memberId && l.getStatus() == LoanStatus::ACTIVE) {
            result.push_back(l);
        }
    }
    return result;
}

std::vector<Loan> FileLoanRepository::findAllByMember(int memberId) {
    std::vector<Loan> result;
    for (const auto& l : loans_) {
        if (l.getMemberId() == memberId) {
            result.push_back(l);
        }
    }
    return result;
}

std::vector<Loan> FileLoanRepository::findAllByBook(int bookId) {
    std::vector<Loan> result;
    for (const auto& l : loans_) {
        if (l.getBookId() == bookId) {
            result.push_back(l);
        }
    }
    return result;
}

std::vector<Loan> FileLoanRepository::getActiveLoans() {
    std::vector<Loan> result;
    for (const auto& l : loans_) {
        if (l.getStatus() == LoanStatus::ACTIVE) {
            result.push_back(l);
        }
    }
    return result;
}

std::vector<Loan> FileLoanRepository::getOverdueLoans() {
    auto now = DateUtils::getCurrentDate();
    std::vector<Loan> result;
    for (const auto& l : loans_) {
        if (l.getStatus() == LoanStatus::ACTIVE && now > l.getDueDate()) {
            result.push_back(l);
        }
    }
    return result;
}

void FileLoanRepository::updateLoan(const Loan& loan) {
    for (auto& l : loans_) {
        if (l.getLoanId() == loan.getLoanId()) {
            l = loan;
            saveToFile();
            return;
        }
    }
}

std::vector<std::pair<int, int>> FileLoanRepository::getMostIssuedBooks(int limit) {
    // Count bookId occurrences across ALL loans (active + returned)
    std::unordered_map<int, int> countMap;
    for (const auto& l : loans_) {
        countMap[l.getBookId()]++;
    }

    // Convert to vector of pairs and sort descending by count
    std::vector<std::pair<int, int>> counts(countMap.begin(), countMap.end());
    std::sort(counts.begin(), counts.end(),
              [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                  return a.second > b.second;
              });

    // Return top `limit`
    if (static_cast<int>(counts.size()) > limit) {
        counts.resize(limit);
    }
    return counts;
}

int FileLoanRepository::getNextId() {
    int maxId = 0;
    for (const auto& l : loans_) {
        if (l.getLoanId() > maxId) maxId = l.getLoanId();
    }
    return maxId + 1;
}
