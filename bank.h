#ifndef BANK_H
#define BANK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ACCOUNTS 100
#define MAX_NAME 50
#define MAX_PASSWORD 20
#define MAX_PHONE 15
#define MAX_EMAIL 50
#define ACCOUNT_FILE "accounts.dat"
#define LOG_DIR "logs"

struct Account {
    int accountNumber;
    char name[MAX_NAME];
    char password[MAX_PASSWORD];
    float balance;
    char phone[MAX_PHONE];
    char email[MAX_EMAIL];
    int isActive;
    char createdOn[24];
};

void clearScreen(void);
void setColor(const char *color);
void printHeader(const char *title);
void printSuccess(const char *message);
void printWarning(const char *message);
void printInfo(const char *message);
void pauseForInput(void);
void readPassword(char *password, size_t size);
int readInt(int *value);
int readFloat(float *value);

int loadAccounts(struct Account accounts[], int *count);
int saveAccounts(const struct Account accounts[], int count);
int createAccount(struct Account accounts[], int *count);
int loginAccount(const struct Account accounts[], int count, struct Account *loggedIn, int *index);
void depositMoney(struct Account accounts[], int count, int index);
void withdrawMoney(struct Account accounts[], int count, int index);
int transferMoney(struct Account accounts[], int count, int index);
void checkBalance(const struct Account *account);
void viewAccountDetails(const struct Account *account);
void viewTransactionHistory(int accountNumber);
int updateAccount(struct Account accounts[], int count, int index);
int deleteAccount(struct Account accounts[], int *count, int index);
int findAccountByNumber(const struct Account accounts[], int count, int accountNumber);
void searchAccounts(const struct Account accounts[], int count);
void sortAccountsByBalance(struct Account accounts[], int count);
void sortAccountsByNumber(struct Account accounts[], int count);
void printAccountList(const struct Account accounts[], int count);
int adminLogin(void);
void adminMenu(struct Account accounts[], int *count);
void userMenu(struct Account accounts[], int *count, int index);
void ensureLogsDirectory(void);
void appendTransaction(int accountNumber, const char *type, float amount, float balance);

#endif
