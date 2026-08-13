#include "bank.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

void setColor(const char *color) {
#ifdef _WIN32
    if (strcmp(color, "BLUE") == 0) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 9);
    else if (strcmp(color, "GREEN") == 0) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
    else if (strcmp(color, "RED") == 0) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
    else if (strcmp(color, "YELLOW") == 0) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
    else if (strcmp(color, "RESET") == 0) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    else SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#else
    if (strcmp(color, "BLUE") == 0) printf("\033[1;34m");
    else if (strcmp(color, "GREEN") == 0) printf("\033[1;32m");
    else if (strcmp(color, "RED") == 0) printf("\033[1;31m");
    else if (strcmp(color, "YELLOW") == 0) printf("\033[1;33m");
    else if (strcmp(color, "RESET") == 0) printf("\033[0m");
    else printf("\033[0m");
#endif
}

void clearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printHeader(const char *title) {
    clearScreen();
    setColor("BLUE");
    printf("\n╔════════════════════════════════════════════╗\n");
    printf("║ %-40s ║\n", title);
    printf("╚════════════════════════════════════════════╝\n");
    setColor("RESET");
}

void printSuccess(const char *message) {
    setColor("GREEN");
    printf("[SUCCESS] %s\n", message);
    setColor("RESET");
}

void printWarning(const char *message) {
    setColor("YELLOW");
    printf("[WARNING] %s\n", message);
    setColor("RESET");
}

void printInfo(const char *message) {
    setColor("BLUE");
    printf("[INFO] %s\n", message);
    setColor("RESET");
}

void pauseForInput(void) {
#ifdef _WIN32
    printf("\nPress Enter to continue...\n");
    getchar();
#else
    if (!isatty(STDIN_FILENO)) {
        return;
    }
    printf("\nPress Enter to continue...\n");
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        ;
    }
#endif
}

void readPassword(char *password, size_t size) {
#ifdef _WIN32
    DWORD mode;
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & ~ENABLE_ECHO_INPUT);
    fgets(password, (int)size, stdin);
    password[strcspn(password, "\n")] = '\0';
    SetConsoleMode(hStdin, mode);
    printf("\n");
#else
    struct termios oldTerm, newTerm;
    tcgetattr(STDIN_FILENO, &oldTerm);
    newTerm = oldTerm;
    newTerm.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newTerm);
    fgets(password, (int)size, stdin);
    password[strcspn(password, "\n")] = '\0';
    tcsetattr(STDIN_FILENO, TCSANOW, &oldTerm);
    printf("\n");
#endif
}

void ensureLogsDirectory(void) {
#ifdef _WIN32
    _mkdir(LOG_DIR);
#else
    struct stat st = {0};
    if (stat(LOG_DIR, &st) == -1) {
        mkdir(LOG_DIR, 0777);
    }
#endif
}

static char *readInputLine(char *buffer, size_t size) {
    if (fgets(buffer, (int)size, stdin) == NULL) {
        return NULL;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    while (buffer[0] == '\0') {
        printf("Input cannot be empty. Please try again: ");
        if (fgets(buffer, (int)size, stdin) == NULL) {
            return NULL;
        }
        buffer[strcspn(buffer, "\n")] = '\0';
    }
    return buffer;
}

int readInt(int *value) {
    char buffer[32];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 0;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    *value = atoi(buffer);
    return 1;
}

int readFloat(float *value) {
    char buffer[32];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 0;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    *value = atof(buffer);
    return 1;
}

static void formatTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

void appendTransaction(int accountNumber, const char *type, float amount, float balance) {
    ensureLogsDirectory();
    char logFile[100];
    char timestamp[24];
    formatTimestamp(timestamp, sizeof(timestamp));
    snprintf(logFile, sizeof(logFile), "%s/%d.txt", LOG_DIR, accountNumber);
    FILE *fp = fopen(logFile, "a");
    if (fp == NULL) {
        perror("Unable to open transaction log");
        return;
    }
    fprintf(fp, "[%s] %s %.2f | Balance: %.2f\n", timestamp, type, amount, balance);
    fclose(fp);
}

int loadAccounts(struct Account accounts[], int *count) {
    FILE *fp = fopen(ACCOUNT_FILE, "rb");
    if (fp == NULL) {
        *count = 0;
        return 0;
    }
    int loaded = 0;
    while (fread(&accounts[loaded], sizeof(struct Account), 1, fp) == 1) {
        if (accounts[loaded].isActive == 1) {
            loaded++;
        }
        if (loaded >= MAX_ACCOUNTS) break;
    }
    fclose(fp);
    *count = loaded;
    return loaded;
}

int saveAccounts(const struct Account accounts[], int count) {
    FILE *fp = fopen(ACCOUNT_FILE, "wb");
    if (fp == NULL) {
        perror("Unable to save accounts");
        return 0;
    }
    for (int i = 0; i < count; ++i) {
        fwrite(&accounts[i], sizeof(struct Account), 1, fp);
    }
    fclose(fp);
    return 1;
}

static int nextAccountNumber(const struct Account accounts[], int count) {
    int highest = 1000;
    for (int i = 0; i < count; ++i) {
        if (accounts[i].accountNumber > highest) {
            highest = accounts[i].accountNumber;
        }
    }
    return highest;
}

int createAccount(struct Account accounts[], int *count) {
    if (*count >= MAX_ACCOUNTS) {
        printWarning("Maximum account limit reached.");
        return 0;
    }

    struct Account newAccount;
    memset(&newAccount, 0, sizeof(newAccount));
    newAccount.accountNumber = nextAccountNumber(accounts, *count) + 1;
    newAccount.isActive = 1;

    printf("Enter full name: ");
    if (readInputLine(newAccount.name, sizeof(newAccount.name)) == NULL) {
        printWarning("Failed to read full name.");
        return 0;
    }
    printf("Enter phone number: ");
    if (readInputLine(newAccount.phone, sizeof(newAccount.phone)) == NULL) {
        printWarning("Failed to read phone number.");
        return 0;
    }
    printf("Enter email: ");
    if (readInputLine(newAccount.email, sizeof(newAccount.email)) == NULL) {
        printWarning("Failed to read email.");
        return 0;
    }

    float initialDeposit = 0.0f;
    printf("Enter initial deposit amount (0 if none): ");
    if (!readFloat(&initialDeposit)) {
        printWarning("Failed to read initial deposit amount.");
        return 0;
    }
    if (initialDeposit < 0) {
        printWarning("Initial deposit cannot be negative.");
        return 0;
    }

    printf("Create password: ");
    readPassword(newAccount.password, sizeof(newAccount.password));

    printf("Confirm password: ");
    char confirm[20];
    readPassword(confirm, sizeof(confirm));
    if (strcmp(newAccount.password, confirm) != 0) {
        printWarning("Passwords do not match. Account not created.");
        return 0;
    }

    newAccount.balance = initialDeposit;
    formatTimestamp(newAccount.createdOn, sizeof(newAccount.createdOn));
    accounts[*count] = newAccount;
    (*count)++;

    ensureLogsDirectory();
    char logFile[100];
    snprintf(logFile, sizeof(logFile), "%s/%d.txt", LOG_DIR, newAccount.accountNumber);
    FILE *fp = fopen(logFile, "w");
    if (fp != NULL) {
        fprintf(fp, "Account created on %s\n", newAccount.createdOn);
        if (initialDeposit > 0) {
            fprintf(fp, "Initial deposit: %.2f\n", initialDeposit);
        }
        fclose(fp);
    }

    if (!saveAccounts(accounts, *count)) {
        return 0;
    }

    printf("\n");
    printSuccess("Account created successfully.");
    printf("Your account number is: %d\n", newAccount.accountNumber);
    return 1;
}

int loginAccount(const struct Account accounts[], int count, struct Account *loggedIn, int *index) {
    int acctNo;
    char password[MAX_PASSWORD];
    printf("Enter account number: ");
    if (!readInt(&acctNo)) {
        return 0;
    }
    printf("Enter password: ");
    readPassword(password, sizeof(password));

    for (int i = 0; i < count; ++i) {
        if (accounts[i].accountNumber == acctNo && accounts[i].isActive == 1 && strcmp(accounts[i].password, password) == 0) {
            *loggedIn = accounts[i];
            *index = i;
            return 1;
        }
    }
    return 0;
}

void depositMoney(struct Account accounts[], int count, int index) {
    float amount;
    printf("Enter amount to deposit: ");
    if (!readFloat(&amount)) {
        printWarning("Failed to read deposit amount.");
        return;
    }
    if (amount <= 0) {
        printWarning("Deposit amount must be positive.");
        return;
    }
    accounts[index].balance += amount;
    appendTransaction(accounts[index].accountNumber, "Deposit", amount, accounts[index].balance);
    saveAccounts(accounts, count);
    printSuccess("Deposit completed successfully.");
}

void withdrawMoney(struct Account accounts[], int count, int index) {
    float amount;
    printf("Enter amount to withdraw: ");
    if (!readFloat(&amount)) {
        printWarning("Failed to read withdrawal amount.");
        return;
    }
    if (amount <= 0) {
        printWarning("Withdrawal amount must be positive.");
        return;
    }
    if (amount > accounts[index].balance) {
        printWarning("Insufficient balance.");
        return;
    }
    accounts[index].balance -= amount;
    appendTransaction(accounts[index].accountNumber, "Withdrawal", amount, accounts[index].balance);
    saveAccounts(accounts, count);
    printSuccess("Withdrawal completed successfully.");
}

int transferMoney(struct Account accounts[], int count, int index) {
    int targetAccountNumber;
    float amount;
    printf("Enter destination account number: ");
    if (!readInt(&targetAccountNumber)) {
        printWarning("Failed to read destination account number.");
        return 0;
    }
    printf("Enter amount to transfer: ");
    if (!readFloat(&amount)) {
        printWarning("Failed to read transfer amount.");
        return 0;
    }

    if (amount <= 0) {
        printWarning("Transfer amount must be positive.");
        return 0;
    }
    if (amount > accounts[index].balance) {
        printWarning("Insufficient balance for transfer.");
        return 0;
    }

    int targetIndex = findAccountByNumber(accounts, count, targetAccountNumber);
    if (targetIndex == -1 || targetIndex == index) {
        printWarning("Destination account is invalid.");
        return 0;
    }

    accounts[index].balance -= amount;
    accounts[targetIndex].balance += amount;
    appendTransaction(accounts[index].accountNumber, "Transfer Out", amount, accounts[index].balance);
    appendTransaction(accounts[targetIndex].accountNumber, "Transfer In", amount, accounts[targetIndex].balance);
    saveAccounts(accounts, count);
    printSuccess("Transfer completed successfully.");
    return 1;
}

void checkBalance(const struct Account *account) {
    printf("Current balance: %.2f\n", account->balance);
}

void viewAccountDetails(const struct Account *account) {
    printf("\nAccount Details\n");
    printf("Account Number : %d\n", account->accountNumber);
    printf("Name           : %s\n", account->name);
    printf("Phone          : %s\n", account->phone);
    printf("Email          : %s\n", account->email);
    printf("Balance        : %.2f\n", account->balance);
    printf("Created On     : %s\n", account->createdOn);
}

void viewTransactionHistory(int accountNumber) {
    char logFile[100];
    snprintf(logFile, sizeof(logFile), "%s/%d.txt", LOG_DIR, accountNumber);
    FILE *fp = fopen(logFile, "r");
    if (fp == NULL) {
        printWarning("No transaction history available yet.");
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        printf("%s", line);
    }
    fclose(fp);
}

int updateAccount(struct Account accounts[], int count, int index) {
    int choice;
    printf("1. Update name\n2. Update phone\n3. Update email\n4. Update password\n5. Cancel\nChoose: ");
    if (!readInt(&choice)) {
        printWarning("Failed to read update choice.");
        return 0;
    }

    switch (choice) {
        case 1:
            printf("Enter new name: ");
            if (readInputLine(accounts[index].name, sizeof(accounts[index].name)) == NULL) {
                printWarning("Name update failed.");
                return 0;
            }
            break;
        case 2:
            printf("Enter new phone: ");
            if (readInputLine(accounts[index].phone, sizeof(accounts[index].phone)) == NULL) {
                printWarning("Phone update failed.");
                return 0;
            }
            break;
        case 3:
            printf("Enter new email: ");
            if (readInputLine(accounts[index].email, sizeof(accounts[index].email)) == NULL) {
                printWarning("Email update failed.");
                return 0;
            }
            break;
        case 4:
            printf("Enter new password: ");
            readPassword(accounts[index].password, sizeof(accounts[index].password));
            break;
        default:
            printWarning("Update cancelled.");
            return 0;
    }
    saveAccounts(accounts, count);
    printSuccess("Account updated successfully.");
    return 1;
}

int deleteAccount(struct Account accounts[], int *count, int index) {
    if (index < 0 || index >= *count) return 0;
    accounts[index].isActive = 0;
    (*count)--;
    if (!saveAccounts(accounts, *count)) return 0;
    printWarning("Account has been deactivated.");
    return 1;
}

int findAccountByNumber(const struct Account accounts[], int count, int accountNumber) {
    for (int i = 0; i < count; ++i) {
        if (accounts[i].accountNumber == accountNumber) return i;
    }
    return -1;
}

void searchAccounts(const struct Account accounts[], int count) {
    char query[50];
    printf("Enter name or account number to search: ");
    if (fgets(query, sizeof(query), stdin) == NULL) {
        return;
    }
    query[strcspn(query, "\n")] = '\0';

    int found = 0;
    for (int i = 0; i < count; ++i) {
        if (strstr(accounts[i].name, query) != NULL || atoi(query) == accounts[i].accountNumber) {
            printf("%d | %s | Balance: %.2f | Active: %s\n",
                   accounts[i].accountNumber,
                   accounts[i].name,
                   accounts[i].balance,
                   accounts[i].isActive ? "Yes" : "No");
            found = 1;
        }
    }
    if (!found) {
        printWarning("No matching account found.");
    }
}

void sortAccountsByBalance(struct Account accounts[], int count) {
    for (int i = 0; i < count - 1; ++i) {
        for (int j = 0; j < count - i - 1; ++j) {
            if (accounts[j].balance < accounts[j + 1].balance) {
                struct Account temp = accounts[j];
                accounts[j] = accounts[j + 1];
                accounts[j + 1] = temp;
            }
        }
    }
}

void sortAccountsByNumber(struct Account accounts[], int count) {
    for (int i = 0; i < count - 1; ++i) {
        for (int j = 0; j < count - i - 1; ++j) {
            if (accounts[j].accountNumber > accounts[j + 1].accountNumber) {
                struct Account temp = accounts[j];
                accounts[j] = accounts[j + 1];
                accounts[j + 1] = temp;
            }
        }
    }
}

void printAccountList(const struct Account accounts[], int count) {
    printf("\nList of accounts:\n");
    for (int i = 0; i < count; ++i) {
        printf("%d | %s | Balance: %.2f | Active: %s\n",
               accounts[i].accountNumber,
               accounts[i].name,
               accounts[i].balance,
               accounts[i].isActive ? "Yes" : "No");
    }
}

int adminLogin(void) {
    const char *adminPassword = "admin123";
    char entered[20];
    printf("Enter admin password: ");
    readPassword(entered, sizeof(entered));
    return strcmp(entered, adminPassword) == 0;
}

void adminMenu(struct Account accounts[], int *count) {
    if (!adminLogin()) {
        printWarning("Invalid admin password.");
        return;
    }

    int choice;
    do {
        printHeader("ADMIN PANEL");
        printf("1. View all accounts\n2. Search account\n3. Sort by balance\n4. Sort by account number\n5. Delete account\n6. Exit\nChoose: ");
        if (!readInt(&choice)) {
            printWarning("Failed to read admin choice.");
            break;
        }

        switch (choice) {
            case 1: {
                printAccountList(accounts, *count);
                break;
            }
            case 2: {
                searchAccounts(accounts, *count);
                break;
            }
            case 3: {
                sortAccountsByBalance(accounts, *count);
                printAccountList(accounts, *count);
                break;
            }
            case 4: {
                sortAccountsByNumber(accounts, *count);
                printAccountList(accounts, *count);
                break;
            }
            case 5: {
                int accNo;
                printf("Enter account number to delete: ");
                if (!readInt(&accNo)) {
                    printWarning("Failed to read account number.");
                    break;
                }
                int idx = findAccountByNumber(accounts, *count, accNo);
                if (idx != -1) {
                    deleteAccount(accounts, count, idx);
                } else {
                    printWarning("Account not found.");
                }
                break;
            }
            case 6:
                printInfo("Returning to main menu.");
                break;
            default:
                printWarning("Invalid choice.");
                break;
        }
        pauseForInput();
    } while (choice != 6);
}

void userMenu(struct Account accounts[], int *count, int index) {
    int choice;
    do {
        printHeader("USER DASHBOARD");
        printf("Account: %d | Name: %s\n", accounts[index].accountNumber, accounts[index].name);
        printf("1. Deposit\n2. Withdraw\n3. Transfer Funds\n4. Check Balance\n5. View Transaction History\n6. View Account Details\n7. Update Account\n8. Logout\nChoose: ");
        if (!readInt(&choice)) {
            printWarning("Failed to read menu choice.");
            break;
        }

        switch (choice) {
            case 1:
                depositMoney(accounts, *count, index);
                break;
            case 2:
                withdrawMoney(accounts, *count, index);
                break;
            case 3:
                transferMoney(accounts, *count, index);
                break;
            case 4:
                checkBalance(&accounts[index]);
                break;
            case 5:
                viewTransactionHistory(accounts[index].accountNumber);
                break;
            case 6:
                viewAccountDetails(&accounts[index]);
                break;
            case 7:
                updateAccount(accounts, *count, index);
                break;
            case 8:
                printInfo("Logged out successfully.");
                break;
            default:
                printWarning("Invalid choice.");
                break;
        }
        pauseForInput();
    } while (choice != 8);
}
