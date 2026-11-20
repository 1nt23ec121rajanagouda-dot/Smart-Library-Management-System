#pragma once

#include <string>
#include <optional>
#include <sstream>
#include <chrono>

enum class LoanStatus {
    ACTIVE,
    RETURNED
};

inline std::string loanStatusToString(LoanStatus s) {
    return (s == LoanStatus::ACTIVE) ? "ACTIVE" : "RETURNED";
}

inline LoanStatus stringToLoanStatus(const std::string& s) {
    return (s == "RETURNED") ? LoanStatus::RETURNED : LoanStatus::ACTIVE;
}

class Loan {
public:
    Loan() = default;

    Loan(int loanId, int bookId, int memberId, int issuedBy,
         std::chrono::system_clock::time_point issueDate,
         std::chrono::system_clock::time_point dueDate)
        : loanId_(loanId), bookId_(bookId), memberId_(memberId),
          issuedBy_(issuedBy), issueDate_(issueDate), dueDate_(dueDate),
          status_(LoanStatus::ACTIVE), fineAmount_(0.0) {}

    // ── Getters ────────────────────────────────────────────────────────────
    int getLoanId()      const { return loanId_; }
    int getBookId()      const { return bookId_; }
    int getMemberId()    const { return memberId_; }
    int getIssuedBy()    const { return issuedBy_; }
    std::chrono::system_clock::time_point getIssueDate()  const { return issueDate_; }
    std::chrono::system_clock::time_point getDueDate()    const { return dueDate_; }
    std::optional<std::chrono::system_clock::time_point> getReturnDate() const { return returnDate_; }
    LoanStatus getStatus()   const { return status_; }
    double getFineAmount()   const { return fineAmount_; }

    // ── Setters ────────────────────────────────────────────────────────────
    void setLoanId(int id)      { loanId_ = id; }
    void setBookId(int id)      { bookId_ = id; }
    void setMemberId(int id)    { memberId_ = id; }
    void setIssuedBy(int id)    { issuedBy_ = id; }
    void setIssueDate(std::chrono::system_clock::time_point t)  { issueDate_ = t; }
    void setDueDate(std::chrono::system_clock::time_point t)    { dueDate_ = t; }
    void setReturnDate(std::optional<std::chrono::system_clock::time_point> t) { returnDate_ = t; }
    void setStatus(LoanStatus s)   { status_ = s; }
    void setFineAmount(double a)   { fineAmount_ = a; }

    // ── Serialization (pipe-delimited) ─────────────────────────────────────
    std::string serialize() const {
        std::ostringstream oss;
        oss << loanId_ << "|"
            << bookId_ << "|"
            << memberId_ << "|"
            << issuedBy_ << "|"
            << std::chrono::system_clock::to_time_t(issueDate_) << "|"
            << std::chrono::system_clock::to_time_t(dueDate_) << "|"
            << (returnDate_ ? std::to_string(std::chrono::system_clock::to_time_t(*returnDate_)) : "") << "|"
            << loanStatusToString(status_) << "|"
            << fineAmount_;
        return oss.str();
    }

    static Loan deserialize(const std::string& line) {
        Loan l;
        std::istringstream iss(line);
        std::string token;

        auto next = [&]() -> std::string {
            std::getline(iss, token, '|');
            return token;
        };

        l.loanId_    = std::stoi(next());
        l.bookId_    = std::stoi(next());
        l.memberId_  = std::stoi(next());
        l.issuedBy_  = std::stoi(next());
        l.issueDate_ = std::chrono::system_clock::from_time_t(std::stoll(next()));
        l.dueDate_   = std::chrono::system_clock::from_time_t(std::stoll(next()));

        std::string retStr = next();
        l.returnDate_ = retStr.empty()
            ? std::nullopt
            : std::optional<std::chrono::system_clock::time_point>(
                  std::chrono::system_clock::from_time_t(std::stoll(retStr)));

        l.status_     = stringToLoanStatus(next());
        l.fineAmount_ = std::stod(next());

        return l;
    }

private:
    int loanId_ = 0;
    int bookId_ = 0;
    int memberId_ = 0;
    int issuedBy_ = 0;
    std::chrono::system_clock::time_point issueDate_;
    std::chrono::system_clock::time_point dueDate_;
    std::optional<std::chrono::system_clock::time_point> returnDate_;
    LoanStatus status_ = LoanStatus::ACTIVE;
    double fineAmount_ = 0.0;
};
