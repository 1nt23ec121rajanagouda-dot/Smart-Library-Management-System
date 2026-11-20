#include <iostream>
#include <memory>
#include "utils/Config.h"
#include "repositories/FileBookRepository.h"
#include "repositories/FileMemberRepository.h"
#include "repositories/FileLoanRepository.h"
#include "repositories/FileAdminRepository.h"
#include "services/AuthService.h"
#include "services/BookService.h"
#include "services/MemberService.h"
#include "services/LoanService.h"
#include "cli/MenuHandler.h"

int main() {
    Config::instance().load();

    std::string storageMode = Config::instance().getStorageMode();

    std::cout << "========================================\n";
    std::cout << "   Smart Library Management System\n";
    std::cout << "========================================\n";
    std::cout << "Storage mode: " << storageMode << "\n\n";

    // Create file-based repositories
    FileBookRepository bookRepo;
    FileMemberRepository memberRepo;
    FileLoanRepository loanRepo;
    FileAdminRepository adminRepo;

    // Create services with dependency injection
    AuthService authService(adminRepo);
    BookService bookService(bookRepo, loanRepo);
    MemberService memberService(memberRepo, loanRepo);
    LoanService loanService(bookRepo, memberRepo, loanRepo);

    // Create and run the menu
    MenuHandler menu(authService, bookService, memberService, loanService);
    menu.run();

    std::cout << "\nThank you for using Smart Library Management System. Goodbye!\n";
    return 0;
}
