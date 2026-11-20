#pragma once

#include <vector>
#include "models/Book.h"
#include "models/Member.h"
#include "models/Loan.h"
#include "services/AuthService.h"
#include "services/BookService.h"
#include "services/MemberService.h"
#include "services/LoanService.h"

class MenuHandler {
public:
    MenuHandler(AuthService& auth, BookService& book,
                MemberService& member, LoanService& loan);

    void run();

private:
    // ── Setup & Login ──────────────────────────────────────────────────────
    void setupFirstAdmin();
    bool loginPrompt();

    // ── Menus ──────────────────────────────────────────────────────────────
    void mainMenu();
    void bookManagementMenu();
    void memberManagementMenu();
    void searchReportsMenu();

    // ── Book operations ────────────────────────────────────────────────────
    void addBook();
    void listAllBooks();
    void searchBooks();
    void updateBook();
    void deactivateBook();

    // ── Member operations ──────────────────────────────────────────────────
    void registerMember();
    void listAllMembers();
    void searchMembers();
    void updateMember();
    void deactivateMember();

    // ── Loan operations ────────────────────────────────────────────────────
    void issueBook();
    void returnBook();

    // ── Reports ────────────────────────────────────────────────────────────
    void showAvailableBooks();
    void showActiveLoans();
    void showOverdueLoans();
    void showMemberHistory();
    void showMostIssuedBooks();
    void showInventorySummary();

    // ── Display helpers ────────────────────────────────────────────────────
    void displayBooks(const std::vector<Book>& books);
    void displayMembers(const std::vector<Member>& members);
    void displayLoans(const std::vector<Loan>& loans);

    // ── Services ───────────────────────────────────────────────────────────
    AuthService& authService_;
    BookService& bookService_;
    MemberService& memberService_;
    LoanService& loanService_;
};
