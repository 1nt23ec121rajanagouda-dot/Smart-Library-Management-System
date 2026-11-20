-- ============================================================================
-- Smart Library Management System - Seed Data
-- Run AFTER schema.sql
-- ============================================================================

USE library_management;

-- ============================================================================
-- Sample Books (6 books across various categories)
-- ============================================================================
INSERT INTO books (title, author, isbn, category, publication_year, total_copies, available_copies, is_active)
VALUES
    ('To Kill a Mockingbird',       'Harper Lee',           '978-0061120084', 'Fiction',    1960, 5, 4, 1),
    ('A Brief History of Time',     'Stephen Hawking',      '978-0553380163', 'Science',    1988, 3, 3, 1),
    ('Clean Code',                  'Robert C. Martin',     '978-0132350884', 'Technology', 2008, 4, 3, 1),
    ('Sapiens',                     'Yuval Noah Harari',    '978-0062316097', 'History',    2011, 3, 2, 1),
    ('The Great Gatsby',            'F. Scott Fitzgerald',  '978-0743273565', 'Fiction',    1925, 2, 2, 1),
    ('Introduction to Algorithms',  'Thomas H. Cormen',     '978-0262033848', 'Technology', 2009, 3, 3, 1);

-- ============================================================================
-- Sample Members (4 members)
-- ============================================================================
INSERT INTO members (name, email, phone, is_active)
VALUES
    ('Alice Johnson',   'alice.johnson@example.com',    '+1-555-0101', 1),
    ('Bob Smith',       'bob.smith@example.com',        '+1-555-0102', 1),
    ('Carol Williams',  'carol.williams@example.com',   '+1-555-0103', 1),
    ('David Brown',     'david.brown@example.com',      '+1-555-0104', 1);

-- ============================================================================
-- Admin user
-- NOTE: The first admin account is created on first run via the application.
-- The app will prompt for a username and password, hash the password with
-- PasswordUtils::hashPassword(), and store it in the admin_users table.
-- Do NOT insert admin credentials directly into this seed file.
-- ============================================================================

-- ============================================================================
-- Sample Loans (3 loan records)
--
-- Prerequisites: at least one admin_users row must exist for issued_by.
-- If you want to test loans without running the app first, uncomment the
-- temporary admin insert below (password hash = bcrypt of "admin123"):
--
-- INSERT INTO admin_users (username, password_hash, is_active)
-- VALUES ('admin', '$2b$12$LJ3m3zRkEUm5V0xGqHrCRu7gN.Zl5e0K1pDmYvOIqR5W9bCfLz5T6', 1);
--
-- The loans below reference issued_by = 1 (the first admin).
-- ============================================================================

-- Loan 1: Alice borrowed "To Kill a Mockingbird", still active
INSERT INTO loans (book_id, member_id, issued_by, issue_date, due_date, return_date, status, fine_amount)
VALUES (1, 1, 1,
        DATE_SUB(CURRENT_TIMESTAMP, INTERVAL 7 DAY),
        DATE_ADD(CURRENT_TIMESTAMP, INTERVAL 7 DAY),
        NULL,
        'ACTIVE',
        0.00);

-- Loan 2: Bob borrowed "Clean Code", already returned
INSERT INTO loans (book_id, member_id, issued_by, issue_date, due_date, return_date, status, fine_amount)
VALUES (3, 2, 1,
        DATE_SUB(CURRENT_TIMESTAMP, INTERVAL 20 DAY),
        DATE_SUB(CURRENT_TIMESTAMP, INTERVAL 6 DAY),
        DATE_SUB(CURRENT_TIMESTAMP, INTERVAL 8 DAY),
        'RETURNED',
        0.00);

-- Loan 3: Carol borrowed "Sapiens", still active
INSERT INTO loans (book_id, member_id, issued_by, issue_date, due_date, return_date, status, fine_amount)
VALUES (4, 3, 1,
        DATE_SUB(CURRENT_TIMESTAMP, INTERVAL 3 DAY),
        DATE_ADD(CURRENT_TIMESTAMP, INTERVAL 11 DAY),
        NULL,
        'ACTIVE',
        0.00);
