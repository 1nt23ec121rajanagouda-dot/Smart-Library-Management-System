#include "services/BookService.h"
#include "utils/DateUtils.h"

BookService::BookService(IBookRepository& bookRepo, ILoanRepository& loanRepo)
    : bookRepo_(bookRepo), loanRepo_(loanRepo) {}

std::pair<bool, std::string> BookService::addBook(const std::string& title,
                                                   const std::string& author,
                                                   const std::string& isbn,
                                                   const std::string& category,
                                                   int year,
                                                   int copies) {
    if (title.empty()) {
        return {false, "Title cannot be empty."};
    }
    if (author.empty()) {
        return {false, "Author cannot be empty."};
    }
    if (category.empty()) {
        return {false, "Category cannot be empty."};
    }
    if (copies <= 0) {
        return {false, "Number of copies must be positive."};
    }

    // Check ISBN uniqueness if provided
    if (!isbn.empty()) {
        auto existingBook = bookRepo_.findByISBN(isbn);
        if (existingBook.has_value()) {
            return {false, "A book with this ISBN already exists."};
        }
    }

    Book book;
    book.setBookId(bookRepo_.getNextId());
    book.setTitle(title);
    book.setAuthor(author);
    if (!isbn.empty()) {
        book.setIsbn(isbn);
    }
    book.setCategory(category);
    if (year > 0) {
        book.setPublicationYear(year);
    }
    book.setTotalCopies(copies);
    book.setAvailableCopies(copies);
    book.setIsActive(true);
    book.setCreatedAt(DateUtils::getCurrentDate());
    book.setUpdatedAt(DateUtils::getCurrentDate());

    bookRepo_.addBook(book);

    return {true, "Book added successfully with ID: " + std::to_string(book.getBookId()) + "."};
}

std::vector<Book> BookService::getAllBooks() {
    return bookRepo_.getAllBooks();
}

std::vector<Book> BookService::getActiveBooks() {
    return bookRepo_.getActiveBooks();
}

std::vector<Book> BookService::getAvailableBooks() {
    auto activeBooks = bookRepo_.getActiveBooks();
    std::vector<Book> available;
    for (const auto& book : activeBooks) {
        if (book.getAvailableCopies() > 0) {
            available.push_back(book);
        }
    }
    return available;
}

std::vector<Book> BookService::searchBooks(const std::string& query, const std::string& type) {
    if (type == "title") {
        return bookRepo_.findByTitle(query);
    } else if (type == "author") {
        return bookRepo_.findByAuthor(query);
    } else if (type == "isbn") {
        auto result = bookRepo_.findByISBN(query);
        if (result.has_value()) {
            return {result.value()};
        }
        return {};
    } else if (type == "category") {
        return bookRepo_.findByCategory(query);
    }
    return {};
}

std::pair<bool, std::string> BookService::updateBook(int id,
                                                      const std::string& title,
                                                      const std::string& author,
                                                      const std::string& isbn,
                                                      const std::string& category,
                                                      int year,
                                                      int totalCopies) {
    auto bookOpt = bookRepo_.findById(id);
    if (!bookOpt.has_value()) {
        return {false, "Book not found."};
    }

    Book book = bookOpt.value();

    if (title.empty()) {
        return {false, "Title cannot be empty."};
    }
    if (author.empty()) {
        return {false, "Author cannot be empty."};
    }
    if (category.empty()) {
        return {false, "Category cannot be empty."};
    }
    if (totalCopies <= 0) {
        return {false, "Total copies must be positive."};
    }

    // Check ISBN uniqueness if changed
    if (!isbn.empty()) {
        auto existingBook = bookRepo_.findByISBN(isbn);
        if (existingBook.has_value() && existingBook->getBookId() != id) {
            return {false, "Another book with this ISBN already exists."};
        }
    }

    // Calculate the difference in total copies and adjust available copies
    int copyDifference = totalCopies - book.getTotalCopies();
    int newAvailable = book.getAvailableCopies() + copyDifference;
    if (newAvailable < 0) {
        return {false, "Cannot reduce total copies below the number currently on loan."};
    }

    book.setTitle(title);
    book.setAuthor(author);
    if (!isbn.empty()) {
        book.setIsbn(isbn);
    }
    book.setCategory(category);
    if (year > 0) {
        book.setPublicationYear(year);
    }
    book.setTotalCopies(totalCopies);
    book.setAvailableCopies(newAvailable);
    book.setUpdatedAt(DateUtils::getCurrentDate());

    bookRepo_.updateBook(book);

    return {true, "Book updated successfully."};
}

std::pair<bool, std::string> BookService::deactivateBook(int id) {
    auto bookOpt = bookRepo_.findById(id);
    if (!bookOpt.has_value()) {
        return {false, "Book not found."};
    }

    // Check for active loans
    auto activeLoans = loanRepo_.findAllByBook(id);
    for (const auto& loan : activeLoans) {
        if (loan.getStatus() == LoanStatus::ACTIVE) {
            return {false, "Cannot deactivate book. There are active loans for this book."};
        }
    }

    bookRepo_.deactivateBook(id);

    return {true, "Book deactivated successfully."};
}
