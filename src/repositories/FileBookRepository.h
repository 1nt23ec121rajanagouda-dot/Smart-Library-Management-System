#pragma once

#include "repositories/IBookRepository.h"
#include <string>
#include <vector>

class FileBookRepository : public IBookRepository {
public:
    explicit FileBookRepository(const std::string& dataDir = "data");
    ~FileBookRepository() override = default;

    void addBook(const Book& book) override;
    std::optional<Book> findById(int bookId) override;
    std::optional<Book> findByISBN(const std::string& isbn) override;
    std::vector<Book> findByTitle(const std::string& title) override;
    std::vector<Book> findByAuthor(const std::string& author) override;
    std::vector<Book> findByCategory(const std::string& category) override;
    std::vector<Book> getAllBooks() override;
    std::vector<Book> getActiveBooks() override;
    void updateBook(const Book& book) override;
    void deactivateBook(int bookId) override;
    int getNextId() override;

private:
    std::string filePath_;
    std::vector<Book> books_;

    void loadFromFile();
    void saveToFile();
    static std::string toLowerStr(const std::string& s);
};
