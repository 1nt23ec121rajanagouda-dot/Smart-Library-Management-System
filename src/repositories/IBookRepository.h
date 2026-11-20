#pragma once

#include "models/Book.h"
#include <vector>
#include <optional>
#include <string>
#include <memory>

class IBookRepository {
public:
    virtual ~IBookRepository() = default;

    virtual void addBook(const Book& book) = 0;
    virtual std::optional<Book> findById(int bookId) = 0;
    virtual std::optional<Book> findByISBN(const std::string& isbn) = 0;
    virtual std::vector<Book> findByTitle(const std::string& title) = 0;
    virtual std::vector<Book> findByAuthor(const std::string& author) = 0;
    virtual std::vector<Book> findByCategory(const std::string& category) = 0;
    virtual std::vector<Book> getAllBooks() = 0;
    virtual std::vector<Book> getActiveBooks() = 0;
    virtual void updateBook(const Book& book) = 0;
    virtual void deactivateBook(int bookId) = 0;
    virtual int getNextId() = 0;
};
