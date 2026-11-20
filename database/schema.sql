-- ============================================================================
-- Smart Library Management System - Database Schema
-- MySQL 8.0+ required
-- ============================================================================

CREATE DATABASE IF NOT EXISTS library_management
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE library_management;

-- ============================================================================
-- Table: books
-- ============================================================================
CREATE TABLE IF NOT EXISTS books (
    book_id         INT             NOT NULL AUTO_INCREMENT,
    title           VARCHAR(255)    NOT NULL,
    author          VARCHAR(255)    NOT NULL,
    isbn            VARCHAR(20)     DEFAULT NULL,
    category        VARCHAR(100)    NOT NULL,
    publication_year INT            DEFAULT NULL,
    total_copies    INT             NOT NULL DEFAULT 1,
    available_copies INT            NOT NULL DEFAULT 1,
    is_active       TINYINT(1)      NOT NULL DEFAULT 1,
    created_at      TIMESTAMP       NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at      TIMESTAMP       NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,

    PRIMARY KEY (book_id),
    UNIQUE KEY uq_books_isbn (isbn),

    CONSTRAINT chk_total_copies_non_negative   CHECK (total_copies >= 0),
    CONSTRAINT chk_available_copies_non_negative CHECK (available_copies >= 0),
    CONSTRAINT chk_available_lte_total         CHECK (available_copies <= total_copies)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Indexes for common search patterns
CREATE INDEX idx_books_title    ON books (title);
CREATE INDEX idx_books_author   ON books (author);
CREATE INDEX idx_books_isbn     ON books (isbn);
CREATE INDEX idx_books_category ON books (category);

-- ============================================================================
-- Table: members
-- ============================================================================
CREATE TABLE IF NOT EXISTS members (
    member_id       INT             NOT NULL AUTO_INCREMENT,
    name            VARCHAR(255)    NOT NULL,
    email           VARCHAR(255)    NOT NULL,
    phone           VARCHAR(20)     NOT NULL,
    registered_at   TIMESTAMP       NOT NULL DEFAULT CURRENT_TIMESTAMP,
    is_active       TINYINT(1)      NOT NULL DEFAULT 1,

    PRIMARY KEY (member_id),
    UNIQUE KEY uq_members_email (email)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Index for email lookups
CREATE INDEX idx_members_email ON members (email);

-- ============================================================================
-- Table: admin_users
-- ============================================================================
CREATE TABLE IF NOT EXISTS admin_users (
    user_id         INT             NOT NULL AUTO_INCREMENT,
    username        VARCHAR(100)    NOT NULL,
    password_hash   VARCHAR(255)    NOT NULL,
    is_active       TINYINT(1)      NOT NULL DEFAULT 1,

    PRIMARY KEY (user_id),
    UNIQUE KEY uq_admin_username (username)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
-- Table: loans
-- ============================================================================
CREATE TABLE IF NOT EXISTS loans (
    loan_id         INT             NOT NULL AUTO_INCREMENT,
    book_id         INT             NOT NULL,
    member_id       INT             NOT NULL,
    issued_by       INT             NOT NULL,
    issue_date      TIMESTAMP       NOT NULL DEFAULT CURRENT_TIMESTAMP,
    due_date        TIMESTAMP       NOT NULL,
    return_date     TIMESTAMP       NULL DEFAULT NULL,
    status          ENUM('ACTIVE', 'RETURNED') NOT NULL DEFAULT 'ACTIVE',
    fine_amount     DECIMAL(10, 2)  NOT NULL DEFAULT 0.00,

    PRIMARY KEY (loan_id),

    CONSTRAINT fk_loans_book
        FOREIGN KEY (book_id)   REFERENCES books (book_id)
        ON UPDATE CASCADE
        ON DELETE RESTRICT,

    CONSTRAINT fk_loans_member
        FOREIGN KEY (member_id) REFERENCES members (member_id)
        ON UPDATE CASCADE
        ON DELETE RESTRICT,

    CONSTRAINT fk_loans_admin
        FOREIGN KEY (issued_by) REFERENCES admin_users (user_id)
        ON UPDATE CASCADE
        ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Indexes for loan queries
CREATE INDEX idx_loans_book_id   ON loans (book_id);
CREATE INDEX idx_loans_member_id ON loans (member_id);
CREATE INDEX idx_loans_status    ON loans (status);
CREATE INDEX idx_loans_due_date  ON loans (due_date);
