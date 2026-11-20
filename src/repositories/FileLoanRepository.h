#pragma once

#include "repositories/ILoanRepository.h"
#include "utils/DateUtils.h"
#include <string>
#include <vector>
#include <utility>

class FileLoanRepository : public ILoanRepository {
public:
    explicit FileLoanRepository(const std::string& dataDir = "data");
    ~FileLoanRepository() override = default;

    void createLoan(const Loan& loan) override;
    std::optional<Loan> findById(int loanId) override;
    std::vector<Loan> findActiveByMember(int memberId) override;
    std::vector<Loan> findAllByMember(int memberId) override;
    std::vector<Loan> findAllByBook(int bookId) override;
    std::vector<Loan> getActiveLoans() override;
    std::vector<Loan> getOverdueLoans() override;
    void updateLoan(const Loan& loan) override;
    std::vector<std::pair<int, int>> getMostIssuedBooks(int limit = 10) override;
    int getNextId() override;

private:
    std::string filePath_;
    std::vector<Loan> loans_;

    void loadFromFile();
    void saveToFile();
};
