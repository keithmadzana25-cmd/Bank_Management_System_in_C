# Bank Management System in C — BlueBank™ (ncurses GUI)

## Abstract

This project presents the design and implementation of a terminal-based Bank Management System written in C, featuring an enhanced ncurses GUI with a blue-themed interface. It automates essential banking processes such as account creation, secure authentication, deposits, withdrawals, and detailed transaction history tracking. Customer records are stored persistently in a binary file (`accounts.dat`), while each account maintains a dated transaction log (`trans_<acc>.txt`). The system demonstrates structured programming, modular file handling, and practical security measures appropriate for an educational but robust project.

## Introduction

The Bank Management System replaces slow, paper-based banking workflows by providing a fast, reliable console application that works across Unix-like systems. It offers a clear visual interface using `ncurses`, password masking, color themes, and centered menus for an outstanding user experience while maintaining simple, auditable file-backed storage.

## Objectives

- Aim: To design and implement a secure, user-friendly Bank Management System that automates account operations using C and file handling.
- Specific objectives:
	- Implement account creation, modification, and deletion.
	- Support deposit, withdrawal, and transaction history features.
	- Ensure data integrity and confidentiality via password authentication.
	- Provide an accessible, color-coded console UI optimized for students and small deployments.

## Technologies Used

- C Programming Language
- File handling (binary `accounts.dat` and text transaction logs)
- `ncurses` for the console GUI and color handling
- `time.h` for timestamps
- Build with `gcc` / `make`

## System Design and Architecture

### Data Structure

The system uses a single `Account` struct to store account metadata and balance. Each record is fixed-size and written to `accounts.dat` as a binary struct. Deleted accounts are marked inactive to keep file operations simple.

Fields include:
- Account number
- Name, gender, date of birth, email, address
- Password (stored locally in plain text for this educational demo — see Security Notes)
- Balance
- Active flag

### Files

- `accounts.dat` — binary file containing `Account` records
- `trans_<acc>.txt` — plain-text transaction log per account

### Console Interface

Blue background theme via `ncurses` color pairs.
Clear centered headers, masked password entry, and simple numeric menus for navigation.

## Features

- Create new bank accounts (automatic account number generator)
- Secure sign-in using account number + password (masked)
- Deposit and withdraw with validation and balance updates
- View per-account transaction history (timestamped)
- Delete accounts (marks inactive and removes transaction file)

## Build & Run

Prerequisites (Linux):

```
sudo apt-get install build-essential libncurses5-dev libncursesw5-dev
```

Build and run:

```
make
./bank_system
```

Notes: The program uses `ncurses`; if it cannot initialize colors it will still run but without the color theme.

## Project Structure

- `src/bank_system.c` — main implementation (ncurses UI, file I/O)
- `accounts.dat` — generated at runtime when creating accounts
- `trans_<acc>.txt` — transaction logs generated per account
- `Makefile` — build helper

## Security Notes and Limitations

This project is an educational prototype. Passwords are stored locally in plain text; for production use, apply proper password hashing (e.g., bcrypt), encryption for files, and networked authentication. File operations are kept simple for clarity.

## Future Work

- Add password hashing and secure storage
- Add role-based access (admin/customer)
- Migrate to a small embedded database (SQLite)
- Add automated tests and CI pipeline

---
Made as an extensible, student-friendly demonstration of file-backed banking operations with a polished terminal UI.
