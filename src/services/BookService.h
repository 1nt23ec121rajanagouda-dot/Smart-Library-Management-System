#pragma once

#include <string>
#include <vector>
#include <utility>
#include <optional>
#include "models/Book.h"
#include "repositories/IBookRepository.h"
#include "repositories/ILoanRepository.h"

class BookService {
public:
    BookService(IBookRepository& bookRepo, ILoanRepository& loanRepo);

    std::pair<bool, std::string> addBook(const std::string& title,
                                         const std::string& author,
                                         const std::string& isbn,
                                         const std::string& category,
                                         int year,
                                         int copies);

    std::vector<Book> getAllBooks();
    std::vector<Book> getActiveBooks();
    std::vector<Book> getAvailableBooks();
    std::vector<Book> searchBooks(const std::string& query, const std::string& type);

    std::pair<bool, std::string> updateBook(int id,
                                            const std::string& title,
                                            const std::string& author,
                                            const std::string& isbn,
                                            const std::string& category,
                                            int year,
                                            int totalCopies);

    std::pair<bool, std::string> deactivateBook(int id);

private:
    IBookRepository& bookRepo_;
    ILoanRepository& loanRepo_;
};
