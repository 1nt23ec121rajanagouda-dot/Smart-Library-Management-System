#pragma once

#include "models/Loan.h"
#include <vector>
#include <optional>
#include <string>
#include <utility>

class ILoanRepository {
public:
    virtual ~ILoanRepository() = default;

    virtual void createLoan(const Loan& loan) = 0;
    virtual std::optional<Loan> findById(int loanId) = 0;
    virtual std::vector<Loan> findActiveByMember(int memberId) = 0;
    virtual std::vector<Loan> findAllByMember(int memberId) = 0;
    virtual std::vector<Loan> findAllByBook(int bookId) = 0;
    virtual std::vector<Loan> getActiveLoans() = 0;
    virtual std::vector<Loan> getOverdueLoans() = 0;
    virtual void updateLoan(const Loan& loan) = 0;

    // Returns pairs of (bookId, issueCount) sorted by count descending
    virtual std::vector<std::pair<int, int>> getMostIssuedBooks(int limit = 10) = 0;

    virtual int getNextId() = 0;
};
