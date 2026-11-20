#include "repositories/FileBookRepository.h"

#include <fstream>
#include <algorithm>
#include <filesystem>
#include <cctype>

FileBookRepository::FileBookRepository(const std::string& dataDir)
    : filePath_(dataDir + "/books.dat") {
    std::filesystem::create_directories(dataDir);
    loadFromFile();
}

// ── Private helpers ────────────────────────────────────────────────────────

void FileBookRepository::loadFromFile() {
    books_.clear();
    std::ifstream in(filePath_);
    if (!in.is_open()) return;          // file not found → empty collection

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            books_.push_back(Book::deserialize(line));
        }
    }
}

void FileBookRepository::saveToFile() {
    std::ofstream out(filePath_, std::ios::trunc);
    for (const auto& book : books_) {
        out << book.serialize() << "\n";
    }
}

std::string FileBookRepository::toLowerStr(const std::string& s) {
    std::string lower;
    lower.reserve(s.size());
    for (char c : s) lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return lower;
}

// ── Interface implementation ───────────────────────────────────────────────

void FileBookRepository::addBook(const Book& book) {
    books_.push_back(book);
    saveToFile();
}

std::optional<Book> FileBookRepository::findById(int bookId) {
    for (const auto& b : books_) {
        if (b.getBookId() == bookId) return b;
    }
    return std::nullopt;
}

std::optional<Book> FileBookRepository::findByISBN(const std::string& isbn) {
    std::string lowerIsbn = toLowerStr(isbn);
    for (const auto& b : books_) {
        if (b.getIsbn().has_value() && toLowerStr(b.getIsbn().value()) == lowerIsbn) {
            return b;
        }
    }
    return std::nullopt;
}

std::vector<Book> FileBookRepository::findByTitle(const std::string& title) {
    std::string lowerTitle = toLowerStr(title);
    std::vector<Book> result;
    for (const auto& b : books_) {
        if (toLowerStr(b.getTitle()).find(lowerTitle) != std::string::npos) {
            result.push_back(b);
        }
    }
    return result;
}

std::vector<Book> FileBookRepository::findByAuthor(const std::string& author) {
    std::string lowerAuthor = toLowerStr(author);
    std::vector<Book> result;
    for (const auto& b : books_) {
        if (toLowerStr(b.getAuthor()).find(lowerAuthor) != std::string::npos) {
            result.push_back(b);
        }
    }
    return result;
}

std::vector<Book> FileBookRepository::findByCategory(const std::string& category) {
    std::string lowerCat = toLowerStr(category);
    std::vector<Book> result;
    for (const auto& b : books_) {
        if (toLowerStr(b.getCategory()).find(lowerCat) != std::string::npos) {
            result.push_back(b);
        }
    }
    return result;
}

std::vector<Book> FileBookRepository::getAllBooks() {
    return books_;
}

std::vector<Book> FileBookRepository::getActiveBooks() {
    std::vector<Book> result;
    for (const auto& b : books_) {
        if (b.getIsActive()) result.push_back(b);
    }
    return result;
}

void FileBookRepository::updateBook(const Book& book) {
    for (auto& b : books_) {
        if (b.getBookId() == book.getBookId()) {
            b = book;
            saveToFile();
            return;
        }
    }
}

void FileBookRepository::deactivateBook(int bookId) {
    for (auto& b : books_) {
        if (b.getBookId() == bookId) {
            b.setIsActive(false);
            saveToFile();
            return;
        }
    }
}

int FileBookRepository::getNextId() {
    int maxId = 0;
    for (const auto& b : books_) {
        if (b.getBookId() > maxId) maxId = b.getBookId();
    }
    return maxId + 1;
}
