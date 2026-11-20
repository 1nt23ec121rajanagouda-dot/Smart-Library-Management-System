#pragma once

#include <string>
#include <vector>
#include <utility>
#include <optional>
#include "models/Loan.h"
#include "repositories/IBookRepository.h"
#include "repositories/IMemberRepository.h"
#include "repositories/ILoanRepository.h"

class LoanService {
public:
    LoanService(IBookRepository& bookRepo,
                IMemberRepository& memberRepo,
                ILoanRepository& loanRepo);

    std::pair<bool, std::string> issueBook(int bookId, int memberId, int issuedBy);
    std::pair<bool, std::string> returnBook(int loanId);

    std::vector<Loan> getActiveLoans();
    std::vector<Loan> getOverdueLoans();
    std::vector<Loan> getMemberHistory(int memberId);

    std::pair<bool, std::string> getEstimatedFine(int loanId);
    std::vector<std::pair<int, int>> getMostIssuedBooks(int limit = 10);

private:
    IBookRepository& bookRepo_;
    IMemberRepository& memberRepo_;
    ILoanRepository& loanRepo_;
};
