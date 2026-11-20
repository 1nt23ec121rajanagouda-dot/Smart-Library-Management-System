// Smart Library Management System — Unit Tests
// Self-contained: no external test framework required.
// Build: make library_tests  |  Run: ./library_tests

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <functional>

#include "utils/DateUtils.h"
#include "utils/PasswordUtils.h"
#include "utils/InputValidator.h"
#include "repositories/FileBookRepository.h"
#include "repositories/FileMemberRepository.h"
#include "repositories/FileLoanRepository.h"
#include "repositories/FileAdminRepository.h"
#include "services/AuthService.h"
#include "services/BookService.h"
#include "services/MemberService.h"
#include "services/LoanService.h"
#include "utils/Config.h"

namespace fs = std::filesystem;

// ── Test harness ──────────────────────────────────────────────────────────────

static int g_passed = 0;
static int g_failed = 0;
static int g_total  = 0;

#define RUN_TEST(name) do { \
    g_total++; \
    try { \
        name(); \
        g_passed++; \
        std::cout << "  [PASS] " << #name << "\n"; \
    } catch (const std::exception& e) { \
        g_failed++; \
        std::cout << "  [FAIL] " << #name << ": " << e.what() << "\n"; \
    } catch (...) { \
        g_failed++; \
        std::cout << "  [FAIL] " << #name << ": unknown exception\n"; \
    } \
} while (0)

#define ASSERT_TRUE(expr) \
    if (!(expr)) throw std::runtime_error("ASSERT_TRUE failed: " #expr)

#define ASSERT_FALSE(expr) \
    if (expr) throw std::runtime_error("ASSERT_FALSE failed: " #expr)

#define ASSERT_EQ(a, b) do { \
    auto _a = (a); auto _b = (b); \
    if (_a != _b) { \
        throw std::runtime_error( \
            std::string("ASSERT_EQ failed: ") + #a + " != " + #b); \
    } \
} while (0)

#define ASSERT_NE(a, b) do { \
    auto _a = (a); auto _b = (b); \
    if (_a == _b) { \
        throw std::runtime_error( \
            std::string("ASSERT_NE failed: ") + #a + " == " + #b); \
    } \
} while (0)

// Create and clean up a unique temp directory per test group
struct TempDir {
    fs::path path;
    explicit TempDir(const std::string& suffix) {
        path = fs::temp_directory_path() / ("library_test_" + suffix);
        fs::create_directories(path);
    }
    ~TempDir() {
        try { fs::remove_all(path); } catch (...) {}
    }
    std::string str() const { return path.string(); }
};

// ── DateUtils Tests ───────────────────────────────────────────────────────────

void test_parseDate_valid() {
    auto tp = DateUtils::parseDate("2024-01-15");
    // Should not throw; verify roundtrip
    ASSERT_EQ(DateUtils::formatDate(tp), "2024-01-15");
}

void test_parseDate_invalid() {
    bool threw = false;
    try { DateUtils::parseDate("not-a-date"); }
    catch (const std::invalid_argument&) { threw = true; }
    ASSERT_TRUE(threw);
}

void test_formatDate_roundtrip() {
    std::string date = "2024-06-15";
    ASSERT_EQ(DateUtils::formatDate(DateUtils::parseDate(date)), date);
}

void test_daysBetween_zero() {
    auto d = DateUtils::parseDate("2024-03-01");
    ASSERT_EQ(DateUtils::daysBetween(d, d), 0);
}

void test_daysBetween_14() {
    auto d1 = DateUtils::parseDate("2024-03-01");
    auto d2 = DateUtils::addDays(d1, 14);
    ASSERT_EQ(DateUtils::daysBetween(d1, d2), 14);
}

void test_addDays_crossMonth() {
    auto d = DateUtils::parseDate("2024-01-01");
    ASSERT_EQ(DateUtils::formatDate(DateUtils::addDays(d, 31)), "2024-02-01");
}

void test_isValidDate_valid() {
    ASSERT_TRUE(DateUtils::isValidDate("2024-01-15"));
}

void test_isValidDate_invalid() {
    ASSERT_FALSE(DateUtils::isValidDate("not-a-date"));
    ASSERT_FALSE(DateUtils::isValidDate("2024-13-01")); // month 13
}

// ── PasswordUtils Tests ───────────────────────────────────────────────────────

void test_hash_deterministic() {
    ASSERT_EQ(PasswordUtils::hashPassword("hello"), PasswordUtils::hashPassword("hello"));
}

void test_hash_different_inputs() {
    ASSERT_NE(PasswordUtils::hashPassword("hello"), PasswordUtils::hashPassword("world"));
}

void test_hash_non_empty() {
    ASSERT_FALSE(PasswordUtils::hashPassword("test").empty());
}

void test_verify_correct() {
    std::string hash = PasswordUtils::hashPassword("mypassword");
    ASSERT_TRUE(PasswordUtils::verifyPassword("mypassword", hash));
}

void test_verify_wrong() {
    std::string hash = PasswordUtils::hashPassword("mypassword");
    ASSERT_FALSE(PasswordUtils::verifyPassword("wrongpassword", hash));
}

// ── InputValidator Tests ──────────────────────────────────────────────────────

void test_isNonEmpty_valid() {
    ASSERT_TRUE(InputValidator::isNonEmpty("hello"));
    ASSERT_TRUE(InputValidator::isNonEmpty("  x  "));
}

void test_isNonEmpty_invalid() {
    ASSERT_FALSE(InputValidator::isNonEmpty(""));
    ASSERT_FALSE(InputValidator::isNonEmpty("   "));
    ASSERT_FALSE(InputValidator::isNonEmpty("\t\n"));
}

void test_isValidEmail_valid() {
    ASSERT_TRUE(InputValidator::isValidEmail("user@example.com"));
    ASSERT_TRUE(InputValidator::isValidEmail("test.user+tag@domain.co.in"));
}

void test_isValidEmail_invalid() {
    ASSERT_FALSE(InputValidator::isValidEmail("notanemail"));
    ASSERT_FALSE(InputValidator::isValidEmail("@nodomain"));
    ASSERT_FALSE(InputValidator::isValidEmail("noatsign.com"));
}

void test_isValidISBN_10() {
    ASSERT_TRUE(InputValidator::isValidISBN("0306406152"));
    ASSERT_TRUE(InputValidator::isValidISBN("0-306-40615-2"));
}

void test_isValidISBN_13() {
    ASSERT_TRUE(InputValidator::isValidISBN("9783161484100"));
    ASSERT_TRUE(InputValidator::isValidISBN("978-3-16-148410-0"));
}

void test_isValidISBN_invalid() {
    ASSERT_FALSE(InputValidator::isValidISBN("12345"));
    ASSERT_FALSE(InputValidator::isValidISBN("abcdefghij"));
}

// ── BookService Tests ─────────────────────────────────────────────────────────

void test_addBook_valid() {
    TempDir tmp("book1");
    FileBookRepository bookRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    BookService svc(bookRepo, loanRepo);

    auto [ok, msg] = svc.addBook("Clean Code", "Robert Martin", "", "Technology", 2008, 3);
    ASSERT_TRUE(ok);

    auto books = svc.getAllBooks();
    ASSERT_EQ(books.size(), size_t(1));
    ASSERT_EQ(books[0].getTitle(), "Clean Code");
    ASSERT_EQ(books[0].getAvailableCopies(), 3);
}

void test_addBook_duplicate_isbn() {
    TempDir tmp("book2");
    FileBookRepository bookRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    BookService svc(bookRepo, loanRepo);

    auto [ok1, _1] = svc.addBook("Book A", "Author A", "1234567890", "Fiction", 2020, 2);
    ASSERT_TRUE(ok1);

    auto [ok2, msg2] = svc.addBook("Book B", "Author B", "1234567890", "Fiction", 2021, 1);
    ASSERT_FALSE(ok2); // duplicate ISBN must be rejected
}

void test_addBook_negative_copies() {
    TempDir tmp("book3");
    FileBookRepository bookRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    BookService svc(bookRepo, loanRepo);

    auto [ok, msg] = svc.addBook("Bad Book", "Author", "", "Fiction", 2020, -1);
    ASSERT_FALSE(ok); // negative copies rejected
}

void test_searchBooks_found() {
    TempDir tmp("book4");
    FileBookRepository bookRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    BookService svc(bookRepo, loanRepo);

    svc.addBook("The Great Gatsby", "F. Scott Fitzgerald", "", "Fiction", 1925, 2);
    auto results = svc.searchBooks("gatsby", "title");
    ASSERT_FALSE(results.empty());
    ASSERT_EQ(results[0].getTitle(), "The Great Gatsby");
}

void test_searchBooks_not_found() {
    TempDir tmp("book5");
    FileBookRepository bookRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    BookService svc(bookRepo, loanRepo);

    svc.addBook("Existing Book", "Some Author", "", "Fiction", 2000, 1);
    auto results = svc.searchBooks("nonexistentxyz", "title");
    ASSERT_TRUE(results.empty());
}

void test_deactivateBook() {
    TempDir tmp("book6");
    FileBookRepository bookRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    BookService svc(bookRepo, loanRepo);

    svc.addBook("Test Book", "Test Author", "", "Test", 2020, 1);
    auto books = svc.getAllBooks();
    int id = books[0].getBookId();

    auto [ok, msg] = svc.deactivateBook(id);
    ASSERT_TRUE(ok);

    auto active = svc.getActiveBooks();
    ASSERT_TRUE(active.empty()); // deactivated book not in active list
}

// ── MemberService Tests ───────────────────────────────────────────────────────

void test_registerMember_valid() {
    TempDir tmp("mem1");
    FileMemberRepository memberRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    MemberService svc(memberRepo, loanRepo);

    auto [ok, msg] = svc.registerMember("Alice Kumar", "alice@example.com", "9876543210");
    ASSERT_TRUE(ok);

    auto members = svc.getAllMembers();
    ASSERT_EQ(members.size(), size_t(1));
    ASSERT_EQ(members[0].getName(), "Alice Kumar");
}

void test_registerMember_duplicate_email() {
    TempDir tmp("mem2");
    FileMemberRepository memberRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    MemberService svc(memberRepo, loanRepo);

    svc.registerMember("Alice", "same@example.com", "1234567890");
    auto [ok, msg] = svc.registerMember("Bob", "same@example.com", "0987654321");
    ASSERT_FALSE(ok); // duplicate email rejected
}

void test_registerMember_empty_name() {
    TempDir tmp("mem3");
    FileMemberRepository memberRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    MemberService svc(memberRepo, loanRepo);

    auto [ok, msg] = svc.registerMember("", "test@example.com", "1234567890");
    ASSERT_FALSE(ok);
}

void test_deactivateMember() {
    TempDir tmp("mem4");
    FileMemberRepository memberRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    MemberService svc(memberRepo, loanRepo);

    svc.registerMember("Bob Singh", "bob@example.com", "9876543210");
    auto members = svc.getAllMembers();
    int id = members[0].getMemberId();

    auto [ok, msg] = svc.deactivateMember(id);
    ASSERT_TRUE(ok);

    auto active = svc.getActiveMembers();
    ASSERT_TRUE(active.empty());
}

// ── LoanService Tests ─────────────────────────────────────────────────────────

// Helper to seed a book and member and return their IDs
static std::pair<int, int> seedBookAndMember(
    BookService& bsvc, MemberService& msvc)
{
    bsvc.addBook("Sample Book", "Sample Author", "", "Fiction", 2020, 2);
    msvc.registerMember("Test User", "test@lib.com", "9999999999");

    int bookId   = bsvc.getAllBooks()[0].getBookId();
    int memberId = msvc.getAllMembers()[0].getMemberId();
    return {bookId, memberId};
}

void test_issueBook_success() {
    TempDir tmp("loan1");
    FileBookRepository bookRepo(tmp.str());
    FileMemberRepository memberRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    BookService bsvc(bookRepo, loanRepo);
    MemberService msvc(memberRepo, loanRepo);
    LoanService lsvc(bookRepo, memberRepo, loanRepo);

    auto [bookId, memberId] = seedBookAndMember(bsvc, msvc);

    auto [ok, msg] = lsvc.issueBook(bookId, memberId, 1);
    ASSERT_TRUE(ok);

    // Available copies should have decremented
    auto books = bsvc.getAllBooks();
    ASSERT_EQ(books[0].getAvailableCopies(), 1); // was 2, now 1
}

void test_issueBook_no_copies_available() {
    TempDir tmp("loan2");
    FileBookRepository bookRepo(tmp.str());
    FileMemberRepository memberRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    BookService bsvc(bookRepo, loanRepo);
    MemberService msvc(memberRepo, loanRepo);
    LoanService lsvc(bookRepo, memberRepo, loanRepo);

    // Add book with only 1 copy, 2 members
    bsvc.addBook("Rare Book", "Author", "", "Fiction", 2020, 1);
    msvc.registerMember("User One", "one@lib.com", "1111111111");
    msvc.registerMember("User Two", "two@lib.com", "2222222222");

    int bookId = bsvc.getAllBooks()[0].getBookId();
    int m1 = msvc.getAllMembers()[0].getMemberId();
    int m2 = msvc.getAllMembers()[1].getMemberId();

    auto [ok1, _1] = lsvc.issueBook(bookId, m1, 1);
    ASSERT_TRUE(ok1);

    auto [ok2, msg2] = lsvc.issueBook(bookId, m2, 1); // no copies left
    ASSERT_FALSE(ok2);
}

void test_issueBook_inactive_member() {
    TempDir tmp("loan3");
    FileBookRepository bookRepo(tmp.str());
    FileMemberRepository memberRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    BookService bsvc(bookRepo, loanRepo);
    MemberService msvc(memberRepo, loanRepo);
    LoanService lsvc(bookRepo, memberRepo, loanRepo);

    auto [bookId, memberId] = seedBookAndMember(bsvc, msvc);
    msvc.deactivateMember(memberId);

    auto [ok, msg] = lsvc.issueBook(bookId, memberId, 1);
    ASSERT_FALSE(ok); // inactive member rejected
}

void test_returnBook_success() {
    TempDir tmp("loan4");
    FileBookRepository bookRepo(tmp.str());
    FileMemberRepository memberRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    BookService bsvc(bookRepo, loanRepo);
    MemberService msvc(memberRepo, loanRepo);
    LoanService lsvc(bookRepo, memberRepo, loanRepo);

    auto [bookId, memberId] = seedBookAndMember(bsvc, msvc);
    lsvc.issueBook(bookId, memberId, 1);

    auto loans = lsvc.getActiveLoans();
    ASSERT_EQ(loans.size(), size_t(1));
    int loanId = loans[0].getLoanId();

    auto [ok, msg] = lsvc.returnBook(loanId);
    ASSERT_TRUE(ok);

    // Copies restored
    auto books = bsvc.getAllBooks();
    ASSERT_EQ(books[0].getAvailableCopies(), 2); // back to original

    // No more active loans
    ASSERT_TRUE(lsvc.getActiveLoans().empty());
}

void test_returnBook_already_returned() {
    TempDir tmp("loan5");
    FileBookRepository bookRepo(tmp.str());
    FileMemberRepository memberRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    BookService bsvc(bookRepo, loanRepo);
    MemberService msvc(memberRepo, loanRepo);
    LoanService lsvc(bookRepo, memberRepo, loanRepo);

    auto [bookId, memberId] = seedBookAndMember(bsvc, msvc);
    lsvc.issueBook(bookId, memberId, 1);

    auto loans = lsvc.getActiveLoans();
    int loanId = loans[0].getLoanId();

    lsvc.returnBook(loanId); // first return — OK
    auto [ok, msg] = lsvc.returnBook(loanId); // second return — must fail
    ASSERT_FALSE(ok);
}

void test_fine_zero_on_time() {
    TempDir tmp("loan6");
    FileBookRepository bookRepo(tmp.str());
    FileMemberRepository memberRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    BookService bsvc(bookRepo, loanRepo);
    MemberService msvc(memberRepo, loanRepo);
    LoanService lsvc(bookRepo, memberRepo, loanRepo);

    auto [bookId, memberId] = seedBookAndMember(bsvc, msvc);
    lsvc.issueBook(bookId, memberId, 1); // issued now, due in 14 days

    auto loans = lsvc.getActiveLoans();
    int loanId = loans[0].getLoanId();

    auto [ok, msg] = lsvc.returnBook(loanId); // returned immediately → no fine
    ASSERT_TRUE(ok);

    // Fine should be 0
    auto history = lsvc.getMemberHistory(memberId);
    ASSERT_EQ(history.size(), size_t(1));
    ASSERT_EQ(history[0].getFineAmount(), 0.0);
}

void test_fine_calculated_when_late() {
    TempDir tmp("loan7");
    FileBookRepository bookRepo(tmp.str());
    FileMemberRepository memberRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    BookService bsvc(bookRepo, loanRepo);
    MemberService msvc(memberRepo, loanRepo);
    LoanService lsvc(bookRepo, memberRepo, loanRepo);

    // Create a loan with a due date in the past (3 days ago)
    bsvc.addBook("Overdue Book", "Author", "", "Fiction", 2020, 1);
    msvc.registerMember("Late Reader", "late@lib.com", "5555555555");
    int bookId   = bsvc.getAllBooks()[0].getBookId();
    int memberId = msvc.getAllMembers()[0].getMemberId();

    // Issue the book
    lsvc.issueBook(bookId, memberId, 1);
    auto loans = lsvc.getActiveLoans();
    ASSERT_EQ(loans.size(), size_t(1));

    // Manually backdate the due date so it's 3 days overdue
    Loan loan = loans[0];
    auto pastDue = DateUtils::addDays(DateUtils::getCurrentDate(), -3);
    loan.setDueDate(pastDue);
    loanRepo.updateLoan(loan);

    // Now return it → should have fine = 3 * fineRate
    auto [ok, msg] = lsvc.returnBook(loan.getLoanId());
    ASSERT_TRUE(ok);

    auto history = lsvc.getMemberHistory(memberId);
    ASSERT_EQ(history.size(), size_t(1));

    int fineRate = Config::instance().getFineRatePerDay(); // default 5
    double expectedFine = 3.0 * fineRate;
    ASSERT_EQ(history[0].getFineAmount(), expectedFine);
}

void test_issueBook_nonexistent_book() {
    TempDir tmp("loan8");
    FileBookRepository bookRepo(tmp.str());
    FileMemberRepository memberRepo(tmp.str());
    FileLoanRepository loanRepo(tmp.str());
    MemberService msvc(memberRepo, loanRepo);
    LoanService lsvc(bookRepo, memberRepo, loanRepo);

    msvc.registerMember("Test User", "t@t.com", "1234567890");
    int memberId = msvc.getAllMembers()[0].getMemberId();

    auto [ok, msg] = lsvc.issueBook(9999, memberId, 1); // book doesn't exist
    ASSERT_FALSE(ok);
}

// ── AuthService Tests ─────────────────────────────────────────────────────────

void test_auth_firstRun() {
    TempDir tmp("auth1");
    FileAdminRepository adminRepo(tmp.str());
    AuthService auth(adminRepo);
    ASSERT_TRUE(auth.isFirstRun()); // no admins yet
}

void test_auth_createAndLogin() {
    TempDir tmp("auth2");
    FileAdminRepository adminRepo(tmp.str());
    AuthService auth(adminRepo);

    auto [ok, msg] = auth.createAdmin("admin", "password123");
    ASSERT_TRUE(ok);
    ASSERT_FALSE(auth.isFirstRun()); // admin now exists

    ASSERT_TRUE(auth.login("admin", "password123"));
    ASSERT_FALSE(auth.login("admin", "wrongpassword"));
    ASSERT_FALSE(auth.login("nobody", "password123"));
}

void test_auth_passwordTooShort() {
    TempDir tmp("auth3");
    FileAdminRepository adminRepo(tmp.str());
    AuthService auth(adminRepo);

    auto [ok, msg] = auth.createAdmin("admin", "abc"); // < 6 chars
    ASSERT_FALSE(ok);
}

// ── Persistence Test ──────────────────────────────────────────────────────────

void test_persistence_across_reload() {
    TempDir tmp("persist1");

    {
        FileBookRepository bookRepo(tmp.str());
        FileLoanRepository loanRepo(tmp.str());
        BookService svc(bookRepo, loanRepo);
        svc.addBook("Persistent Book", "Author", "9780000000001", "Science", 2022, 5);
    }
    // Reload from same directory
    {
        FileBookRepository bookRepo(tmp.str());
        FileLoanRepository loanRepo(tmp.str());
        BookService svc(bookRepo, loanRepo);
        auto books = svc.getAllBooks();
        ASSERT_EQ(books.size(), size_t(1));
        ASSERT_EQ(books[0].getTitle(), "Persistent Book");
        ASSERT_EQ(books[0].getAvailableCopies(), 5);
    }
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main() {
    // Load config with defaults (no .env needed for tests)
    Config::instance().load();

    std::cout << "\n===== Smart Library Management System — Unit Tests =====\n\n";

    std::cout << "--- DateUtils Tests ---\n";
    RUN_TEST(test_parseDate_valid);
    RUN_TEST(test_parseDate_invalid);
    RUN_TEST(test_formatDate_roundtrip);
    RUN_TEST(test_daysBetween_zero);
    RUN_TEST(test_daysBetween_14);
    RUN_TEST(test_addDays_crossMonth);
    RUN_TEST(test_isValidDate_valid);
    RUN_TEST(test_isValidDate_invalid);

    std::cout << "\n--- PasswordUtils Tests ---\n";
    RUN_TEST(test_hash_deterministic);
    RUN_TEST(test_hash_different_inputs);
    RUN_TEST(test_hash_non_empty);
    RUN_TEST(test_verify_correct);
    RUN_TEST(test_verify_wrong);

    std::cout << "\n--- InputValidator Tests ---\n";
    RUN_TEST(test_isNonEmpty_valid);
    RUN_TEST(test_isNonEmpty_invalid);
    RUN_TEST(test_isValidEmail_valid);
    RUN_TEST(test_isValidEmail_invalid);
    RUN_TEST(test_isValidISBN_10);
    RUN_TEST(test_isValidISBN_13);
    RUN_TEST(test_isValidISBN_invalid);

    std::cout << "\n--- BookService Tests ---\n";
    RUN_TEST(test_addBook_valid);
    RUN_TEST(test_addBook_duplicate_isbn);
    RUN_TEST(test_addBook_negative_copies);
    RUN_TEST(test_searchBooks_found);
    RUN_TEST(test_searchBooks_not_found);
    RUN_TEST(test_deactivateBook);

    std::cout << "\n--- MemberService Tests ---\n";
    RUN_TEST(test_registerMember_valid);
    RUN_TEST(test_registerMember_duplicate_email);
    RUN_TEST(test_registerMember_empty_name);
    RUN_TEST(test_deactivateMember);

    std::cout << "\n--- LoanService Tests ---\n";
    RUN_TEST(test_issueBook_success);
    RUN_TEST(test_issueBook_no_copies_available);
    RUN_TEST(test_issueBook_inactive_member);
    RUN_TEST(test_issueBook_nonexistent_book);
    RUN_TEST(test_returnBook_success);
    RUN_TEST(test_returnBook_already_returned);
    RUN_TEST(test_fine_zero_on_time);
    RUN_TEST(test_fine_calculated_when_late);

    std::cout << "\n--- AuthService Tests ---\n";
    RUN_TEST(test_auth_firstRun);
    RUN_TEST(test_auth_createAndLogin);
    RUN_TEST(test_auth_passwordTooShort);

    std::cout << "\n--- Persistence Tests ---\n";
    RUN_TEST(test_persistence_across_reload);

    std::cout << "\n========================================\n";
    std::cout << "Results: " << g_passed << "/" << g_total << " passed";
    if (g_failed > 0) std::cout << "  (" << g_failed << " FAILED)";
    std::cout << "\n========================================\n\n";

    return g_failed > 0 ? 1 : 0;
}
