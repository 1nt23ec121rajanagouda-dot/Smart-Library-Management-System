# Interview Preparation — Smart Library Management System

---

## 30-Second Pitch

> "I built a Smart Library Management System in C++17 — a fully working console application that manages books, members, loans, and fines. It uses a layered architecture: CLI → Service layer → Repository interfaces → file-backed storage, with an optional MySQL path. I wrote 42 automated unit tests covering every business rule. The project demonstrates encapsulation, abstraction, polymorphism through repository interfaces, dependency injection, RAII, and proper date handling with the STL chrono library."

---

## Architecture Walkthrough

### Layers

```
MenuHandler (CLI)
     ↓  calls
Services: AuthService, BookService, MemberService, LoanService
     ↓  depend on (via constructor injection)
Repository Interfaces: IBookRepository, IMemberRepository, ILoanRepository, IAdminRepository
     ↓  implemented by
File Repositories: FileBookRepository, FileMemberRepository, FileLoanRepository, FileAdminRepository
     ↓  persist to
data/*.dat  (pipe-delimited flat files)
```

**Why this layering?**  
Business rules (e.g., "you cannot issue to an inactive member") live only in services. The menu never talks to files directly. The database is behind interfaces, so swapping from file to MySQL requires changing only `main.cpp` — services and menus don't change.

---

## Class Responsibilities

| Class | File | Responsibility |
|-------|------|----------------|
| `Book` | `src/models/Book.h` | Value object: fields, getters/setters, validation, pipe-delimited serialization |
| `Member` | `src/models/Member.h` | Same pattern for member data |
| `Loan` | `src/models/Loan.h` | Loan lifecycle fields; `LoanStatus` enum (ACTIVE/RETURNED) |
| `Admin` | `src/models/Admin.h` | Admin credentials; never stores plaintext password |
| `IBookRepository` | `src/repositories/IBookRepository.h` | Pure virtual interface — data access contract |
| `FileBookRepository` | `src/repositories/FileBookRepository.*` | Loads/saves `data/books.dat`; in-memory vector |
| `AuthService` | `src/services/AuthService.*` | Login, admin creation, first-run detection |
| `BookService` | `src/services/BookService.*` | Book CRUD + ISBN uniqueness + deactivation safety |
| `MemberService` | `src/services/MemberService.*` | Member CRUD + email uniqueness + deactivation safety |
| `LoanService` | `src/services/LoanService.*` | Issue/return logic, fine calculation, loan reports |
| `MenuHandler` | `src/cli/MenuHandler.*` | All console I/O; delegates entirely to services |
| `DateUtils` | `src/utils/DateUtils.*` | `parseDate`, `formatDate`, `daysBetween`, `addDays` using `std::chrono` |
| `InputValidator` | `src/utils/InputValidator.*` | Email/ISBN/phone regex, safe cin reading with error recovery |
| `Config` | `src/utils/Config.*` | Singleton; reads `.env` file + env vars; typed getters |
| `PasswordUtils` | `src/utils/PasswordUtils.*` | SHA-256 via CommonCrypto; `hashPassword`, `verifyPassword` |

---

## OOP Concepts Actually Used

### 1. Encapsulation
All model fields are `private_` with `public` getters and validated setters.  
Example (`Book.h`):
```cpp
void setAvailableCopies(int n) {
    if (n < 0) throw std::invalid_argument("Available copies cannot be negative");
    if (n > totalCopies_) throw std::invalid_argument("Cannot exceed total copies");
    availableCopies_ = n;
    touch();
}
```

### 2. Abstraction
Repository interfaces (`IBookRepository`, etc.) expose *what* operations exist without revealing *how* data is stored. Services program to the interface, never to `FileBookRepository` directly.

### 3. Polymorphism
`FileBookRepository` and (optionally) `MySQLBookRepository` both implement `IBookRepository`. The service layer holds an `IBookRepository&` — at runtime either implementation works identically.

### 4. Inheritance
All file repositories inherit from their pure-virtual interface:
```cpp
class FileBookRepository : public IBookRepository {
    void addBook(const Book& book) override;
    // ...
};
```

---

## Data Structures & Time Complexities

| Operation | Structure | Complexity | Reason |
|-----------|-----------|------------|--------|
| Load all books | `std::vector<Book>` | O(n) | Full file scan on load |
| Find book by ID | Linear scan of vector | O(n) | Acceptable for academic scale |
| Find by title/author | Linear scan + tolower | O(n) | Case-insensitive search |
| Config lookup | `std::unordered_map` | O(1) avg | Key-value store |
| Most-issued books | Frequency count + sort | O(n log n) | Count then partial sort |
| getNextId | O(n) scan for max | O(n) | Runs once per add |

> For production scale, an unordered_map keyed by ID would give O(1) lookup. The current vector approach is clean and correct for the academic demo scope.

---

## Database Schema & Relationships

```sql
books         (book_id PK, title, author, isbn UNIQUE, category, 
               publication_year, total_copies, available_copies CHECK(>=0),
               is_active, created_at, updated_at)

members       (member_id PK, name, email UNIQUE, phone, registered_at, is_active)

admin_users   (user_id PK, username UNIQUE, password_hash, is_active)

loans         (loan_id PK,
               book_id FK → books.book_id,
               member_id FK → members.member_id,
               issued_by FK → admin_users.user_id,
               issue_date, due_date, return_date NULL,
               status ENUM('ACTIVE','RETURNED'),
               fine_amount, created_at)
```

**Key design choices:**
- `available_copies` stored denormalised in `books` for O(1) availability check
- `loans` keeps history even after book/member deactivation (no cascade delete)
- `fine_amount` stored at return time — avoids recalculating historical fines

---

## Business Rules Implemented

| Rule | Where enforced |
|------|---------------|
| Available copies never negative | `Book::setAvailableCopies()` + `LoanService` |
| Available ≤ total copies | `Book::setAvailableCopies()` validator |
| Cannot issue to inactive member | `LoanService::issueBook()` |
| Cannot issue inactive/unavailable book | `LoanService::issueBook()` |
| Max active loans per member (default 3) | `LoanService::issueBook()` |
| Cannot return already-returned loan | `LoanService::returnBook()` |
| Deactivate book only if no active loans | `BookService::deactivateBook()` |
| Deactivate member only if no active loans | `MemberService::deactivateMember()` |
| ISBN uniqueness (when provided) | `BookService::addBook()` |
| Email uniqueness | `MemberService::registerMember()` |

---

## Fine Calculation

```
overdue_days = max(0, daysBetween(dueDate, returnDate))
fine         = overdue_days × FINE_RATE_PER_DAY   (default: ₹5/day)
```

- Calculated at return time using `DateUtils::daysBetween`
- For still-active overdue loans: `getEstimatedFine()` calculates as-of-today
- Stored as `double fine_amount` on the `Loan` object after return
- Displayed with "Rs." prefix (ASCII-safe fallback for ₹ symbol)

---

## Design Decisions & Tradeoffs

| Decision | Reason |
|----------|--------|
| File-based default storage | No server dependency; runs anywhere; great for demo/interview |
| Pipe-delimited format | Simple to implement, human-readable, no parser library needed |
| Repository interface pattern | Enables swapping file→MySQL without changing service or CLI code |
| Dependency injection via constructor | Testable; services don't create their own dependencies |
| Config singleton | One load point; avoids passing config through many layers |
| `std::optional` for ISBN/year | Correctly models optional fields without sentinel values |
| `std::chrono::system_clock` for dates | Type-safe; avoids string comparison bugs |
| SHA-256 (CommonCrypto) | Available on macOS without dependencies; no plaintext passwords |
| Self-contained test harness | No Catch2/GTest install required; builds and runs anywhere |

---

## Testing Approach

- **Unit tests** in `tests/test_main.cpp` — 42 tests, 0 external dependencies
- Each test creates a **unique temp directory** in `/tmp/library_test_*/`, cleaned up on destruction via RAII `TempDir` struct
- Tests are **isolated** — no shared state between test functions
- Coverage: DateUtils, PasswordUtils, InputValidator, BookService, MemberService, LoanService, AuthService, persistence roundtrip

---

## 25 Likely Interview Questions & Answers

**1. Walk me through your project architecture.**  
Layered: CLI (MenuHandler) → Services (business rules) → Repository interfaces → File implementations. Each layer depends only on the layer below via abstractions, never on concrete implementations.

**2. Why did you use abstract repository interfaces?**  
To separate business logic from storage. Services hold `IBookRepository&` — at runtime they work with file or MySQL equally. This is the Repository pattern; it also makes services unit-testable without a real database.

**3. What is dependency injection and where do you use it?**  
Passing dependencies (repositories) into a class's constructor rather than creating them inside. `BookService(IBookRepository& bookRepo, ILoanRepository& loanRepo)` receives its repos from `main.cpp`, which decides the concrete type.

**4. What is RAII and where do you apply it?**  
Resource Acquisition Is Initialisation — resources are tied to object lifetime. My `TempDir` test helper creates a directory in its constructor and deletes it in its destructor. `FileBookRepository` opens and closes files in RAII style (stack-allocated `std::ifstream`/`std::ofstream`).

**5. How do you prevent SQL injection?**  
All MySQL repositories (guarded by `#ifdef HAS_MYSQL_CONNECTOR`) use prepared statements with parameter binding — user input is never concatenated into SQL strings.

**6. How does your fine calculation work?**  
`overdue_days = max(0, daysBetween(dueDate, returnDate))`. Fine = `overdue_days × FINE_RATE_PER_DAY`. Both values are configurable via `.env`. Calculation uses `std::chrono` — no string comparison.

**7. How do you store passwords securely?**  
SHA-256 hash via CommonCrypto. Only the hex digest is stored. `verifyPassword()` hashes the input and compares digests. No plaintext is ever written to disk.

**8. What C++17 features do you use?**  
`std::optional` (ISBN, publication year, return date), structured bindings (`auto [ok, msg] = service.addBook(...)`), `std::filesystem` (creating `data/` directory), `if constexpr` idiom in template code, `std::string_view` compatibility.

**9. How do you handle concurrent access?**  
The current design is single-user (console app). File repos load-then-save without locking. For concurrency, we'd add `std::mutex` around file I/O or switch to MySQL with `SELECT ... FOR UPDATE` and row-level locking (shown in schema comments).

**10. What is the time complexity of searching for a book by title?**  
O(n) — linear scan of the in-memory vector with case-insensitive substring match. For large catalogues, we'd add an inverted index or use the MySQL `LIKE` index.

**11. Why did you choose pipe-delimited files over JSON/XML/SQLite?**  
Zero external dependencies, human-readable, trivial to implement serialise/deserialise in a weekend project. The tradeoff is no structured queries and manual field escaping if pipes appear in data.

**12. How does your authentication flow work?**  
On first run, `AuthService::isFirstRun()` detects no admins and triggers setup. The user creates a username + password; the password is SHA-256 hashed and stored. Login compares `hashPassword(input) == storedHash`.

**13. What design patterns does your project use?**  
- **Repository pattern** — data access behind interfaces  
- **Singleton** — `Config::instance()`  
- **Dependency Injection** — services receive repos via constructor  
- **Template Method** — base interface defines the contract; subclasses implement it

**14. How does your loan issue flow prevent race conditions?**  
In file mode: single-user, no concurrency. In MySQL mode (provided in schema): `BEGIN TRANSACTION; SELECT available_copies FOR UPDATE; -- check > 0; INSERT loan; UPDATE available_copies - 1; COMMIT;` — row-level locking prevents double-issue.

**15. What happens if a book is deactivated but has active loans?**  
`BookService::deactivateBook()` calls `loanRepo.findAllByBook(id)` and checks for any `ACTIVE` loans. If any exist, it returns `{false, "Cannot deactivate: book has active loans"}`. The book stays active.

**16. How do you ensure available_copies never goes negative?**  
Three-layer defence: (1) `Book::setAvailableCopies()` throws on negative, (2) `LoanService::issueBook()` checks `availableCopies > 0` before issuing, (3) MySQL `CHECK (available_copies >= 0)` constraint.

**17. How did you structure your CMakeLists.txt?**  
One static library `library_core` gathers all `.cpp` from `src/` subdirs. `library_app` links `library_core` + `src/main.cpp`. `library_tests` links `library_core` + `tests/test_main.cpp`. CMake `GLOB_RECURSE` picks up new files automatically.

**18. How do you handle bad user input in the menu?**  
`InputValidator::getValidatedInt()` loops until a valid integer in range is entered. `getValidatedString()` loops until non-empty. After `cin >> int`, we always call `cin.ignore(numeric_limits<streamsize>::max(), '\n')` to clear the buffer.

**19. What is `std::optional` and why do you use it?**  
A type-safe wrapper for a value that may or may not exist. Used for `isbn_`, `publicationYear_`, and `returnDate_` — fields that are genuinely optional. Much safer than sentinel values (`-1`, `""`) because the compiler forces you to handle the absent case.

**20. How do you do case-insensitive search?**  
Convert both the search query and the field value to lowercase using `std::tolower` character-by-character, then use `std::string::find`. See `FileBookRepository::findByTitle()`.

**21. What would you change if you had more time?**  
Add an `std::unordered_map<int, Book>` cache for O(1) ID lookups, write a proper MySQL connector integration, add a reservation queue using `std::queue`, and refactor fine calculation to support per-book fine rates.

**22. How did you test your fine calculation?**  
In `test_fine_calculated_when_late()`: issue a book, manually backdate the loan's `dueDate` to 3 days in the past via `loanRepo.updateLoan()`, then return it. Assert `fineAmount == 3 × fineRatePerDay`.

**23. How does persistence work across restarts?**  
`FileBookRepository` constructor opens `data/books.dat` and deserializes each line into a `Book` via `Book::deserialize()`. On any mutation (add/update/deactivate), it rewrites the entire file. The `test_persistence_across_reload` test verifies this.

**24. What is const-correctness and where is it used?**  
Marking methods and parameters `const` where they don't modify state. All getters: `int getBookId() const`. Repository query methods: `std::vector<Book> getAllBooks()` (logically non-mutating; could be `const` if the in-memory vector were immutable after load). `const std::string&` parameters throughout.

**25. How would you scale this to 1 million books?**  
Switch storage to MySQL with the existing interface. Add indexes on `title`, `author`, `isbn`. Use connection pooling. Paginate list results. Add Redis caching for hot books. Use an Elasticsearch index for full-text search. The repository interface layer means none of these changes touch the service or CLI code.
