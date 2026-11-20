#include "cli/MenuHandler.h"
#include "utils/InputValidator.h"
#include "utils/DateUtils.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <string>

// ============================================================================
//  Constructor
// ============================================================================

MenuHandler::MenuHandler(AuthService& auth, BookService& book,
                         MemberService& member, LoanService& loan)
    : authService_(auth), bookService_(book),
      memberService_(member), loanService_(loan) {}

// ============================================================================
//  run() — main entry point
// ============================================================================

void MenuHandler::run() {
    // First-run admin setup
    if (authService_.isFirstRun()) {
        setupFirstAdmin();
    }

    // Login loop
    while (true) {
        if (loginPrompt()) {
            break;
        }
        std::cout << "Invalid username or password. Please try again.\n\n";
    }

    // Main menu loop
    mainMenu();
}

// ============================================================================
//  First-run admin setup
// ============================================================================

void MenuHandler::setupFirstAdmin() {
    std::cout << "\n========================================\n";
    std::cout << "         First-Time Setup\n";
    std::cout << "========================================\n";
    std::cout << "Welcome! No administrator account exists.\n";
    std::cout << "Please create the first admin account to continue.\n\n";

    std::string username = InputValidator::getValidatedString("Enter admin username: ");

    std::string password;
    std::string confirmPassword;

    while (true) {
        password = InputValidator::getValidatedString("Enter password (min 6 characters): ");
        if (password.size() < 6) {
            std::cout << "Password must be at least 6 characters. Try again.\n";
            continue;
        }
        confirmPassword = InputValidator::getValidatedString("Confirm password: ");
        if (password != confirmPassword) {
            std::cout << "Passwords do not match. Try again.\n";
            continue;
        }
        break;
    }

    auto [success, message] = authService_.createAdmin(username, password);
    if (success) {
        std::cout << "\nAdmin account created successfully!\n\n";
    } else {
        std::cout << "\nError creating admin: " << message << "\n\n";
    }
}

// ============================================================================
//  Login prompt
// ============================================================================

bool MenuHandler::loginPrompt() {
    std::cout << "--- Login ---\n";
    std::string username = InputValidator::getValidatedString("Username: ");
    std::string password = InputValidator::getValidatedString("Password: ");

    return authService_.login(username, password);
}

// ============================================================================
//  Main Menu
// ============================================================================

void MenuHandler::mainMenu() {
    while (true) {
        std::cout << "\n===== Smart Library Management System =====\n";
        std::cout << "1. Book Management\n";
        std::cout << "2. Member Management\n";
        std::cout << "3. Issue Book\n";
        std::cout << "4. Return Book\n";
        std::cout << "5. Search & Reports\n";
        std::cout << "6. Exit\n";

        int choice = InputValidator::getValidatedInt("Enter your choice: ", 1, 6);
        if (std::cin.eof()) return;

        switch (choice) {
            case 1: bookManagementMenu();   break;
            case 2: memberManagementMenu(); break;
            case 3: issueBook();            break;
            case 4: returnBook();           break;
            case 5: searchReportsMenu();    break;
            case 6: return;
            default: std::cout << "Invalid choice. Try again.\n"; break;
        }
    }
}

// ============================================================================
//  Book Management Submenu
// ============================================================================

void MenuHandler::bookManagementMenu() {
    while (true) {
        std::cout << "\n--- Book Management ---\n";
        std::cout << "1. Add Book\n";
        std::cout << "2. List All Books\n";
        std::cout << "3. Search Books\n";
        std::cout << "4. Update Book\n";
        std::cout << "5. Deactivate Book\n";
        std::cout << "6. Back\n";

        int choice = InputValidator::getValidatedInt("Enter your choice: ", 1, 6);
        if (std::cin.eof()) return;

        switch (choice) {
            case 1: addBook();        break;
            case 2: listAllBooks();   break;
            case 3: searchBooks();    break;
            case 4: updateBook();     break;
            case 5: deactivateBook(); break;
            case 6: return;
            default: std::cout << "Invalid choice. Try again.\n"; break;
        }
    }
}

// ============================================================================
//  Member Management Submenu
// ============================================================================

void MenuHandler::memberManagementMenu() {
    while (true) {
        std::cout << "\n--- Member Management ---\n";
        std::cout << "1. Register Member\n";
        std::cout << "2. List All Members\n";
        std::cout << "3. Search Members\n";
        std::cout << "4. Update Member\n";
        std::cout << "5. Deactivate Member\n";
        std::cout << "6. Back\n";

        int choice = InputValidator::getValidatedInt("Enter your choice: ", 1, 6);
        if (std::cin.eof()) return;

        switch (choice) {
            case 1: registerMember();   break;
            case 2: listAllMembers();   break;
            case 3: searchMembers();    break;
            case 4: updateMember();     break;
            case 5: deactivateMember(); break;
            case 6: return;
            default: std::cout << "Invalid choice. Try again.\n"; break;
        }
    }
}

// ============================================================================
//  Search & Reports Submenu
// ============================================================================

void MenuHandler::searchReportsMenu() {
    while (true) {
        std::cout << "\n--- Search & Reports ---\n";
        std::cout << "1. Search Books\n";
        std::cout << "2. Available Books\n";
        std::cout << "3. Active Loans\n";
        std::cout << "4. Overdue Loans\n";
        std::cout << "5. Member History\n";
        std::cout << "6. Most Issued Books\n";
        std::cout << "7. Inventory Summary\n";
        std::cout << "8. Back\n";

        int choice = InputValidator::getValidatedInt("Enter your choice: ", 1, 8);
        if (std::cin.eof()) return;

        switch (choice) {
            case 1: searchBooks();          break;
            case 2: showAvailableBooks();   break;
            case 3: showActiveLoans();      break;
            case 4: showOverdueLoans();     break;
            case 5: showMemberHistory();    break;
            case 6: showMostIssuedBooks();  break;
            case 7: showInventorySummary(); break;
            case 8: return;
            default: std::cout << "Invalid choice. Try again.\n"; break;
        }
    }
}

// ============================================================================
//  Book Operations
// ============================================================================

void MenuHandler::addBook() {
    std::cout << "\n--- Add New Book ---\n";

    std::string title    = InputValidator::getValidatedString("Title: ");
    std::string author   = InputValidator::getValidatedString("Author: ");
    std::string isbn     = InputValidator::getValidatedString("ISBN (or press Enter to skip): ");
    std::string category = InputValidator::getValidatedString("Category: ");
    int year             = InputValidator::getValidatedInt("Publication Year: ", 0, 9999);
    int copies           = InputValidator::getValidatedInt("Number of copies: ", 1, 999999);

    auto [success, message] = bookService_.addBook(title, author, isbn, category, year, copies);
    std::cout << message << "\n";
}

void MenuHandler::listAllBooks() {
    std::cout << "\n--- All Books ---\n";
    displayBooks(bookService_.getAllBooks());
}

void MenuHandler::searchBooks() {
    std::cout << "\n--- Search Books ---\n";
    std::cout << "Search by: 1. Title  2. Author  3. Category\n";
    int searchType = InputValidator::getValidatedInt("Enter search type: ", 1, 3);

    std::string type;
    switch (searchType) {
        case 1: type = "title";    break;
        case 2: type = "author";   break;
        case 3: type = "category"; break;
        default: type = "title";   break;
    }

    std::string query = InputValidator::getValidatedString("Enter search query: ");
    displayBooks(bookService_.searchBooks(query, type));
}

void MenuHandler::updateBook() {
    std::cout << "\n--- Update Book ---\n";

    int id               = InputValidator::getValidatedInt("Enter Book ID: ", 1, 999999);
    std::string title    = InputValidator::getValidatedString("New Title: ");
    std::string author   = InputValidator::getValidatedString("New Author: ");
    std::string isbn     = InputValidator::getValidatedString("New ISBN (or press Enter to skip): ");
    std::string category = InputValidator::getValidatedString("New Category: ");
    int year             = InputValidator::getValidatedInt("New Publication Year: ", 0, 9999);
    int totalCopies      = InputValidator::getValidatedInt("New Total Copies: ", 1, 999999);

    auto [success, message] = bookService_.updateBook(id, title, author, isbn, category, year, totalCopies);
    std::cout << message << "\n";
}

void MenuHandler::deactivateBook() {
    std::cout << "\n--- Deactivate Book ---\n";
    int id = InputValidator::getValidatedInt("Enter Book ID to deactivate: ", 1, 999999);

    if (InputValidator::getConfirmation("Are you sure you want to deactivate this book?")) {
        auto [success, message] = bookService_.deactivateBook(id);
        std::cout << message << "\n";
    } else {
        std::cout << "Operation cancelled.\n";
    }
}

// ============================================================================
//  Member Operations
// ============================================================================

void MenuHandler::registerMember() {
    std::cout << "\n--- Register New Member ---\n";

    std::string name  = InputValidator::getValidatedString("Name: ");
    std::string email = InputValidator::getValidatedString("Email: ");
    std::string phone = InputValidator::getValidatedString("Phone: ");

    auto [success, message] = memberService_.registerMember(name, email, phone);
    std::cout << message << "\n";
}

void MenuHandler::listAllMembers() {
    std::cout << "\n--- All Members ---\n";
    displayMembers(memberService_.getAllMembers());
}

void MenuHandler::searchMembers() {
    std::cout << "\n--- Search Members ---\n";
    std::string query = InputValidator::getValidatedString("Enter name to search: ");
    displayMembers(memberService_.searchMembers(query));
}

void MenuHandler::updateMember() {
    std::cout << "\n--- Update Member ---\n";

    int id            = InputValidator::getValidatedInt("Enter Member ID: ", 1, 999999);
    std::string name  = InputValidator::getValidatedString("New Name: ");
    std::string email = InputValidator::getValidatedString("New Email: ");
    std::string phone = InputValidator::getValidatedString("New Phone: ");

    auto [success, message] = memberService_.updateMember(id, name, email, phone);
    std::cout << message << "\n";
}

void MenuHandler::deactivateMember() {
    std::cout << "\n--- Deactivate Member ---\n";
    int id = InputValidator::getValidatedInt("Enter Member ID to deactivate: ", 1, 999999);

    if (InputValidator::getConfirmation("Are you sure you want to deactivate this member?")) {
        auto [success, message] = memberService_.deactivateMember(id);
        std::cout << message << "\n";
    } else {
        std::cout << "Operation cancelled.\n";
    }
}

// ============================================================================
//  Loan Operations
// ============================================================================

void MenuHandler::issueBook() {
    std::cout << "\n--- Issue Book ---\n";

    int bookId   = InputValidator::getValidatedInt("Enter Book ID: ", 1, 999999);
    int memberId = InputValidator::getValidatedInt("Enter Member ID: ", 1, 999999);
    int issuedBy = authService_.getCurrentAdminId();

    auto [success, message] = loanService_.issueBook(bookId, memberId, issuedBy);
    std::cout << message << "\n";
}

void MenuHandler::returnBook() {
    std::cout << "\n--- Return Book ---\n";

    int loanId = InputValidator::getValidatedInt("Enter Loan ID: ", 1, 999999);

    // Show estimated fine before returning
    auto [fineSuccess, fineMsg] = loanService_.getEstimatedFine(loanId);
    if (fineSuccess) {
        std::cout << "Estimated fine: " << fineMsg << "\n";
    }

    auto [success, message] = loanService_.returnBook(loanId);
    std::cout << message << "\n";
}

// ============================================================================
//  Reports
// ============================================================================

void MenuHandler::showAvailableBooks() {
    std::cout << "\n--- Available Books ---\n";
    displayBooks(bookService_.getAvailableBooks());
}

void MenuHandler::showActiveLoans() {
    std::cout << "\n--- Active Loans ---\n";
    displayLoans(loanService_.getActiveLoans());
}

void MenuHandler::showOverdueLoans() {
    std::cout << "\n--- Overdue Loans ---\n";
    displayLoans(loanService_.getOverdueLoans());
}

void MenuHandler::showMemberHistory() {
    std::cout << "\n--- Member Loan History ---\n";
    int memberId = InputValidator::getValidatedInt("Enter Member ID: ", 1, 999999);
    displayLoans(loanService_.getMemberHistory(memberId));
}

void MenuHandler::showMostIssuedBooks() {
    std::cout << "\n--- Most Issued Books ---\n";
    int limit = InputValidator::getValidatedInt("How many top books to show? ", 1, 100);

    auto results = loanService_.getMostIssuedBooks(limit);
    if (results.empty()) {
        std::cout << "No records found.\n";
        return;
    }

    std::cout << std::left
              << std::setw(12) << "Book ID"
              << std::setw(15) << "Issue Count" << "\n";
    std::cout << std::string(27, '-') << "\n";

    for (const auto& [bookId, count] : results) {
        std::cout << std::left
                  << std::setw(12) << bookId
                  << std::setw(15) << count << "\n";
    }
}

void MenuHandler::showInventorySummary() {
    std::cout << "\n--- Inventory Summary ---\n";

    auto allBooks    = bookService_.getAllBooks();
    auto activeBooks = bookService_.getActiveBooks();
    auto activeLoans = loanService_.getActiveLoans();
    auto overdueLoans = loanService_.getOverdueLoans();

    int totalBooks = 0;
    int totalAvailable = 0;
    for (const auto& book : activeBooks) {
        totalBooks += book.getTotalCopies();
        totalAvailable += book.getAvailableCopies();
    }

    std::cout << "Total book titles:       " << allBooks.size() << "\n";
    std::cout << "Active book titles:      " << activeBooks.size() << "\n";
    std::cout << "Total copies (active):   " << totalBooks << "\n";
    std::cout << "Available copies:        " << totalAvailable << "\n";
    std::cout << "Currently issued:        " << activeLoans.size() << "\n";
    std::cout << "Overdue loans:           " << overdueLoans.size() << "\n";
}

// ============================================================================
//  Display Helpers
// ============================================================================

void MenuHandler::displayBooks(const std::vector<Book>& books) {
    if (books.empty()) {
        std::cout << "No records found.\n";
        return;
    }

    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(25) << "Title"
              << std::setw(20) << "Author"
              << std::setw(16) << "ISBN"
              << std::setw(14) << "Category"
              << std::setw(6)  << "Year"
              << std::setw(7)  << "Total"
              << std::setw(7)  << "Avail"
              << std::setw(10) << "Status"
              << "\n";
    std::cout << std::string(111, '-') << "\n";

    for (const auto& b : books) {
        std::string isbn = b.getIsbn().value_or("-");
        std::string year = b.getPublicationYear()
                               ? std::to_string(*b.getPublicationYear())
                               : "-";
        std::string status = b.getIsActive() ? "Active" : "Inactive";

        std::cout << std::left
                  << std::setw(6)  << b.getBookId()
                  << std::setw(25) << b.getTitle().substr(0, 24) // keep columns tidy
                  << std::setw(20) << b.getAuthor().substr(0, 19)
                  << std::setw(16) << isbn.substr(0, 15)
                  << std::setw(14) << b.getCategory().substr(0, 13)
                  << std::setw(6)  << year
                  << std::setw(7)  << b.getTotalCopies()
                  << std::setw(7)  << b.getAvailableCopies()
                  << std::setw(10) << status
                  << "\n";
    }
}

void MenuHandler::displayMembers(const std::vector<Member>& members) {
    if (members.empty()) {
        std::cout << "No records found.\n";
        return;
    }

    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(22) << "Name"
              << std::setw(25) << "Email"
              << std::setw(16) << "Phone"
              << std::setw(14) << "Registered"
              << std::setw(10) << "Status"
              << "\n";
    std::cout << std::string(93, '-') << "\n";

    for (const auto& m : members) {
        std::string status = m.getIsActive() ? "Active" : "Inactive";

        std::cout << std::left
                  << std::setw(6)  << m.getMemberId()
                  << std::setw(22) << m.getName().substr(0, 21)
                  << std::setw(25) << m.getEmail().substr(0, 24)
                  << std::setw(16) << m.getPhone().substr(0, 15)
                  << std::setw(14) << DateUtils::formatDate(m.getRegisteredAt())
                  << std::setw(10) << status
                  << "\n";
    }
}

void MenuHandler::displayLoans(const std::vector<Loan>& loans) {
    if (loans.empty()) {
        std::cout << "No records found.\n";
        return;
    }

    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(10) << "Book ID"
              << std::setw(12) << "Member ID"
              << std::setw(14) << "Issue Date"
              << std::setw(14) << "Due Date"
              << std::setw(14) << "Return Date"
              << std::setw(10) << "Status"
              << std::setw(10) << "Fine"
              << "\n";
    std::cout << std::string(90, '-') << "\n";

    for (const auto& l : loans) {
        std::string returnDate = l.getReturnDate()
                                     ? DateUtils::formatDate(*l.getReturnDate())
                                     : "-";
        std::string status = loanStatusToString(l.getStatus());

        std::cout << std::left
                  << std::setw(6)  << l.getLoanId()
                  << std::setw(10) << l.getBookId()
                  << std::setw(12) << l.getMemberId()
                  << std::setw(14) << DateUtils::formatDate(l.getIssueDate())
                  << std::setw(14) << DateUtils::formatDate(l.getDueDate())
                  << std::setw(14) << returnDate
                  << std::setw(10) << status
                  << std::setw(10) << std::fixed << std::setprecision(2)
                                   << l.getFineAmount()
                  << "\n";
    }
}
