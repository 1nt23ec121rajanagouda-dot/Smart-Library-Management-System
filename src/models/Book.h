#pragma once

#include <string>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <chrono>

class Book {
public:
    Book() = default;

    Book(int id, const std::string& title, const std::string& author,
         const std::string& category, int totalCopies)
        : bookId_(id), title_(title), author_(author),
          category_(category), totalCopies_(totalCopies),
          availableCopies_(totalCopies), isActive_(true) {
        auto now = std::chrono::system_clock::now();
        createdAt_ = now;
        updatedAt_ = now;
        validate();
    }

    // ── Getters ────────────────────────────────────────────────────────────
    int getBookId()          const { return bookId_; }
    const std::string& getTitle()    const { return title_; }
    const std::string& getAuthor()   const { return author_; }
    const std::optional<std::string>& getIsbn() const { return isbn_; }
    const std::string& getCategory() const { return category_; }
    std::optional<int> getPublicationYear() const { return publicationYear_; }
    int getTotalCopies()     const { return totalCopies_; }
    int getAvailableCopies() const { return availableCopies_; }
    bool getIsActive()       const { return isActive_; }
    std::chrono::system_clock::time_point getCreatedAt() const { return createdAt_; }
    std::chrono::system_clock::time_point getUpdatedAt() const { return updatedAt_; }

    // ── Setters ────────────────────────────────────────────────────────────
    void setBookId(int id)                     { bookId_ = id; touch(); }
    void setTitle(const std::string& t)        { title_ = t; touch(); }
    void setAuthor(const std::string& a)       { author_ = a; touch(); }
    void setIsbn(const std::optional<std::string>& i) { isbn_ = i; touch(); }
    void setCategory(const std::string& c)     { category_ = c; touch(); }
    void setPublicationYear(std::optional<int> y) { publicationYear_ = y; touch(); }

    void setTotalCopies(int n) {
        if (n < 0) throw std::invalid_argument("Total copies cannot be negative");
        totalCopies_ = n;
        if (availableCopies_ > totalCopies_) availableCopies_ = totalCopies_;
        touch();
    }

    void setAvailableCopies(int n) {
        if (n < 0) throw std::invalid_argument("Available copies cannot be negative");
        if (n > totalCopies_) throw std::invalid_argument("Available copies cannot exceed total copies");
        availableCopies_ = n;
        touch();
    }

    void setIsActive(bool a)  { isActive_ = a; touch(); }

    void setCreatedAt(std::chrono::system_clock::time_point t) { createdAt_ = t; }
    void setUpdatedAt(std::chrono::system_clock::time_point t) { updatedAt_ = t; }

    // ── Validation ─────────────────────────────────────────────────────────
    void validate() const {
        if (title_.empty()) throw std::invalid_argument("Title cannot be empty");
        if (author_.empty()) throw std::invalid_argument("Author cannot be empty");
        if (totalCopies_ < 0) throw std::invalid_argument("Total copies cannot be negative");
        if (availableCopies_ < 0) throw std::invalid_argument("Available copies cannot be negative");
        if (availableCopies_ > totalCopies_)
            throw std::invalid_argument("Available copies cannot exceed total copies");
    }

    // ── Serialization (pipe-delimited) ─────────────────────────────────────
    std::string serialize() const {
        std::ostringstream oss;
        oss << bookId_ << "|"
            << title_ << "|"
            << author_ << "|"
            << isbn_.value_or("") << "|"
            << category_ << "|"
            << (publicationYear_ ? std::to_string(*publicationYear_) : "") << "|"
            << totalCopies_ << "|"
            << availableCopies_ << "|"
            << (isActive_ ? 1 : 0) << "|"
            << std::chrono::system_clock::to_time_t(createdAt_) << "|"
            << std::chrono::system_clock::to_time_t(updatedAt_);
        return oss.str();
    }

    static Book deserialize(const std::string& line) {
        Book b;
        std::istringstream iss(line);
        std::string token;

        auto next = [&]() -> std::string {
            std::getline(iss, token, '|');
            return token;
        };

        b.bookId_         = std::stoi(next());
        b.title_          = next();
        b.author_         = next();
        std::string isbnStr = next();
        b.isbn_           = isbnStr.empty() ? std::nullopt : std::optional<std::string>(isbnStr);
        b.category_       = next();
        std::string yearStr = next();
        b.publicationYear_ = yearStr.empty() ? std::nullopt : std::optional<int>(std::stoi(yearStr));
        b.totalCopies_    = std::stoi(next());
        b.availableCopies_ = std::stoi(next());
        b.isActive_       = (next() == "1");
        b.createdAt_      = std::chrono::system_clock::from_time_t(std::stoll(next()));
        b.updatedAt_      = std::chrono::system_clock::from_time_t(std::stoll(next()));

        return b;
    }

private:
    void touch() { updatedAt_ = std::chrono::system_clock::now(); }

    int bookId_ = 0;
    std::string title_;
    std::string author_;
    std::optional<std::string> isbn_;
    std::string category_;
    std::optional<int> publicationYear_;
    int totalCopies_ = 0;
    int availableCopies_ = 0;
    bool isActive_ = true;
    std::chrono::system_clock::time_point createdAt_;
    std::chrono::system_clock::time_point updatedAt_;
};
