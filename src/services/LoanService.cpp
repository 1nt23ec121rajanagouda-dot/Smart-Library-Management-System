#include "services/LoanService.h"
#include "utils/DateUtils.h"
#include "utils/Config.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

LoanService::LoanService(IBookRepository& bookRepo,
                         IMemberRepository& memberRepo,
                         ILoanRepository& loanRepo)
    : bookRepo_(bookRepo), memberRepo_(memberRepo), loanRepo_(loanRepo) {}

std::pair<bool, std::string> LoanService::issueBook(int bookId, int memberId, int issuedBy) {
    // Find book — must exist and be active
    auto bookOpt = bookRepo_.findById(bookId);
    if (!bookOpt.has_value()) {
        return {false, "Book not found."};
    }
    Book book = bookOpt.value();
    if (!book.getIsActive()) {
        return {false, "Book is not active."};
    }

    // Find member — must exist and be active
    auto memberOpt = memberRepo_.findById(memberId);
    if (!memberOpt.has_value()) {
        return {false, "Member not found."};
    }
    Member member = memberOpt.value();
    if (!member.getIsActive()) {
        return {false, "Member is not active."};
    }

    // Check available copies
    if (book.getAvailableCopies() <= 0) {
        return {false, "No copies available for this book."};
    }

    // Check max active loans limit
    auto memberActiveLoans = loanRepo_.findActiveByMember(memberId);
    int maxLoans = Config::instance().getMaxActiveLoans();
    if (static_cast<int>(memberActiveLoans.size()) >= maxLoans) {
        return {false, "Member has reached the maximum number of active loans (" +
                std::to_string(maxLoans) + ")."};
    }

    // Create the loan
    auto now = DateUtils::getCurrentDate();
    int loanPeriod = Config::instance().getLoanPeriodDays();
    auto dueDate = DateUtils::addDays(now, loanPeriod);

    Loan loan;
    loan.setLoanId(loanRepo_.getNextId());
    loan.setBookId(bookId);
    loan.setMemberId(memberId);
    loan.setIssuedBy(issuedBy);
    loan.setIssueDate(now);
    loan.setDueDate(dueDate);
    loan.setStatus(LoanStatus::ACTIVE);
    loan.setFineAmount(0.0);

    // Decrement available copies
    book.setAvailableCopies(book.getAvailableCopies() - 1);
    book.setUpdatedAt(now);
    bookRepo_.updateBook(book);

    // Store the loan
    loanRepo_.createLoan(loan);

    return {true, "Book issued successfully. Loan ID: " + std::to_string(loan.getLoanId()) +
            ". Due date: " + DateUtils::formatDate(dueDate) + "."};
}

std::pair<bool, std::string> LoanService::returnBook(int loanId) {
    // Find loan — must exist and be ACTIVE
    auto loanOpt = loanRepo_.findById(loanId);
    if (!loanOpt.has_value()) {
        return {false, "Loan not found."};
    }
    Loan loan = loanOpt.value();
    if (loan.getStatus() != LoanStatus::ACTIVE) {
        return {false, "This loan has already been returned."};
    }

    auto now = DateUtils::getCurrentDate();
    loan.setReturnDate(now);

    // Calculate overdue days and fine
    int overdueDays = std::max(0, DateUtils::daysBetween(loan.getDueDate(), now));
    double fineRate = Config::instance().getFineRatePerDay();
    double fine = overdueDays * fineRate;

    loan.setStatus(LoanStatus::RETURNED);
    loan.setFineAmount(fine);

    // Increment available copies (capped at totalCopies)
    auto bookOpt = bookRepo_.findById(loan.getBookId());
    if (bookOpt.has_value()) {
        Book book = bookOpt.value();
        int newAvailable = book.getAvailableCopies() + 1;
        if (newAvailable > book.getTotalCopies()) {
            newAvailable = book.getTotalCopies();
        }
        book.setAvailableCopies(newAvailable);
        book.setUpdatedAt(now);
        bookRepo_.updateBook(book);
    }

    // Update the loan
    loanRepo_.updateLoan(loan);

    // Build return message
    std::ostringstream msg;
    msg << "Book returned successfully.";
    if (overdueDays > 0) {
        msg << " Overdue by " << overdueDays << " day(s).";
        msg << std::fixed << std::setprecision(2);
        msg << " Fine: Rs. " << fine << ".";
    } else {
        msg << " No fine incurred.";
    }

    return {true, msg.str()};
}

std::vector<Loan> LoanService::getActiveLoans() {
    return loanRepo_.getActiveLoans();
}

std::vector<Loan> LoanService::getOverdueLoans() {
    return loanRepo_.getOverdueLoans();
}

std::vector<Loan> LoanService::getMemberHistory(int memberId) {
    return loanRepo_.findAllByMember(memberId);
}

std::pair<bool, std::string> LoanService::getEstimatedFine(int loanId) {
    auto loanOpt = loanRepo_.findById(loanId);
    if (!loanOpt.has_value()) {
        return {false, "Loan not found."};
    }

    Loan loan = loanOpt.value();
    if (loan.getStatus() != LoanStatus::ACTIVE) {
        std::ostringstream msg;
        msg << std::fixed << std::setprecision(2);
        msg << "Loan is already returned. Final fine: Rs. " << loan.getFineAmount() << ".";
        return {true, msg.str()};
    }

    auto now = DateUtils::getCurrentDate();
    int overdueDays = std::max(0, DateUtils::daysBetween(loan.getDueDate(), now));
    double fineRate = Config::instance().getFineRatePerDay();
    double estimatedFine = overdueDays * fineRate;

    std::ostringstream msg;
    msg << std::fixed << std::setprecision(2);
    if (overdueDays > 0) {
        msg << "Overdue by " << overdueDays << " day(s). Estimated fine: Rs. " << estimatedFine << ".";
    } else {
        msg << "No fine. Book is not yet overdue.";
    }

    return {true, msg.str()};
}

std::vector<std::pair<int, int>> LoanService::getMostIssuedBooks(int limit) {
    return loanRepo_.getMostIssuedBooks(limit);
}
