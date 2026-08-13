#include "bank.h"

int main(void) {
    struct Account accounts[MAX_ACCOUNTS];
    int count = 0;
    int loaded = loadAccounts(accounts, &count);
    (void)loaded;

    int choice;
    do {
        printHeader("BANK MANAGEMENT SYSTEM");
        setColor("BLUE");
        printf("\n[ MAIN MENU ]\n");
        printf("  1. Create Account\n");
        printf("  2. Login\n");
        printf("  3. Admin Panel\n");
        printf("  4. Exit\n");
        setColor("RESET");
        printf("\nSelect an option: ");
        if (!readInt(&choice)) {
            printWarning("Failed to read menu choice.");
            break;
        }

        switch (choice) {
            case 1: {
                createAccount(accounts, &count);
                break;
            }
            case 2: {
                struct Account loggedIn;
                int index = -1;
                if (loginAccount(accounts, count, &loggedIn, &index)) {
                    userMenu(accounts, &count, index);
                } else {
                    printWarning("Invalid account number or password.");
                }
                break;
            }
            case 3: {
                adminMenu(accounts, &count);
                break;
            }
            case 4: {
                printInfo("Thank you for using the Bank Management System.");
                break;
            }
            default:
                printWarning("Invalid choice. Please choose again.");
                break;
        }
        pauseForInput();
    } while (choice != 4);

    return 0;
}
