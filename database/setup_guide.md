# Smart Library Management System — MySQL Setup Guide

This guide walks you through installing MySQL, creating the database, loading the
schema and seed data, and connecting the application.

---

## Table of Contents

1. [Install MySQL](#1-install-mysql)
2. [Start the MySQL Service](#2-start-the-mysql-service)
3. [Create the Database and User](#3-create-the-database-and-user)
4. [Run the Schema and Seed Scripts](#4-run-the-schema-and-seed-scripts)
5. [Configure the `.env` File](#5-configure-the-env-file)
6. [Test the Connection](#6-test-the-connection)
7. [Troubleshooting](#7-troubleshooting)

---

## 1. Install MySQL

### macOS (Homebrew)

```bash
brew install mysql
```

### Linux (Debian / Ubuntu)

```bash
sudo apt update
sudo apt install mysql-server mysql-client
```

### Linux (RHEL / CentOS / Fedora)

```bash
# RHEL 8+ / Fedora
sudo dnf install mysql-server mysql

# CentOS 7
sudo yum install mysql-server mysql
```

### Windows

1. Download the **MySQL Installer** from <https://dev.mysql.com/downloads/installer/>.
2. Run the installer and choose **Developer Default** (or **Server Only**).
3. Follow the wizard — set a root password when prompted.
4. The installer adds `mysql` to your PATH automatically.

---

## 2. Start the MySQL Service

### macOS

```bash
# Start now and auto-start on boot
brew services start mysql

# Or start once without auto-start
mysql.server start
```

### Linux (systemd)

```bash
sudo systemctl start mysqld      # or mysql, depending on distro
sudo systemctl enable mysqld     # auto-start on boot
```

### Windows

MySQL is registered as a Windows Service during installation. If it isn't
running:

```powershell
net start MySQL80    # service name may differ (MySQL57, MySQL84, etc.)
```

### Secure the installation (all platforms, first time only)

```bash
mysql_secure_installation
```

Follow the prompts to set a root password, remove anonymous users, disallow
remote root login, and drop the test database.

---

## 3. Create the Database and User

Connect as root:

```bash
mysql -u root -p
```

Then run the following SQL to create a dedicated user with **least privileges**:

```sql
-- Create the database (the schema script also does this, but it's safe to run twice)
CREATE DATABASE IF NOT EXISTS library_management
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

-- Create a dedicated application user
CREATE USER IF NOT EXISTS 'library_app'@'localhost'
    IDENTIFIED BY 'StrongP@ssw0rd!';   -- ← change this!

-- Grant only the privileges the app needs
GRANT SELECT, INSERT, UPDATE, DELETE
    ON library_management.*
    TO 'library_app'@'localhost';

FLUSH PRIVILEGES;
```

> **Note:** Replace `'StrongP@ssw0rd!'` with a strong, unique password. Never
> use the root account for the application.

---

## 4. Run the Schema and Seed Scripts

From the project root directory, load the schema first, then the seed data:

```bash
# Load schema
mysql -u root -p < database/schema.sql

# Load demo data
mysql -u root -p < database/seed.sql
```

> **Important:** The seed data for loans references `issued_by = 1`, which
> requires at least one row in `admin_users`. The app creates the first admin on
> its initial run. If you want seed loans to work before running the app,
> uncomment the temporary admin insert in `seed.sql`.

Verify the tables were created:

```bash
mysql -u library_app -p -e "USE library_management; SHOW TABLES;"
```

Expected output:

```
+-------------------------------+
| Tables_in_library_management  |
+-------------------------------+
| admin_users                   |
| books                         |
| loans                         |
| members                       |
+-------------------------------+
```

---

## 5. Configure the `.env` File

Create a `.env` file in the project root (if one doesn't already exist):

```dotenv
# Database connection
DB_HOST=127.0.0.1
DB_PORT=3306
DB_NAME=library_management
DB_USER=library_app
DB_PASSWORD=StrongP@ssw0rd!

# Application settings
LOAN_PERIOD_DAYS=14
FINE_RATE_PER_DAY=1.00
MAX_ACTIVE_LOANS=3
STORAGE_MODE=mysql
```

| Variable           | Description                                       | Default       |
|--------------------|---------------------------------------------------|---------------|
| `DB_HOST`          | MySQL server hostname or IP                       | `127.0.0.1`   |
| `DB_PORT`          | MySQL server port                                 | `3306`        |
| `DB_NAME`          | Database name                                     | `library_management` |
| `DB_USER`          | MySQL user for the app                            | `library_app` |
| `DB_PASSWORD`      | Password for the above user                       | *(set yours)* |
| `LOAN_PERIOD_DAYS` | Default loan duration in days                     | `14`          |
| `FINE_RATE_PER_DAY`| Fine charged per overdue day (currency units)     | `1.00`        |
| `MAX_ACTIVE_LOANS` | Max books a member can borrow at once             | `3`           |
| `STORAGE_MODE`     | `mysql` for database, `file` for JSON file storage| `mysql`       |

> **Security:** Add `.env` to your `.gitignore` so credentials are never committed.

---

## 6. Test the Connection

Verify the app user can connect and query:

```bash
mysql -u library_app -p library_management -e "SELECT COUNT(*) AS book_count FROM books;"
```

If the seed data was loaded, you should see:

```
+------------+
| book_count |
+------------+
|          6 |
+------------+
```

You can also run the application directly:

```bash
./build/library_management
```

The app will prompt you to create an admin account on first launch.

---

## 7. Troubleshooting

### "Access denied for user"
- Double-check the username and password in `.env`.
- Make sure `FLUSH PRIVILEGES;` was run after creating the user.
- Verify the user was created for the correct host (`'localhost'` vs `'%'`).

### "Can't connect to MySQL server"
- Ensure the MySQL service is running (`brew services list`, `systemctl status mysqld`).
- Check that `DB_HOST` and `DB_PORT` match your server configuration.
- On Linux, check if MySQL is listening on TCP (`sudo ss -tlnp | grep 3306`).

### "Table doesn't exist"
- Run `schema.sql` before `seed.sql`.
- Make sure you are connecting to the `library_management` database.

### "CHECK constraint violated" on insert
- `available_copies` must be ≤ `total_copies` and both must be ≥ 0.
- Verify your seed data respects these constraints.

### Character encoding issues
- The schema uses `utf8mb4`. Ensure your MySQL server's default character set
  matches, or connect with `--default-character-set=utf8mb4`.
