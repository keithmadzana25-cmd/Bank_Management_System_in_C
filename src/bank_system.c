/*
 * Bank Management System (ncurses-based)
 * Compile: make
 * Requires: ncurses
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>
#include <unistd.h>

#define ACC_FILE "accounts.dat"
#define MAX_NAME 50
#define MAX_EMAIL 80
#define MAX_ADDR 128

typedef struct Account {
    int acc_no;
    char name[MAX_NAME];
    char gender[8];
    char dob[16];
    char email[MAX_EMAIL];
    char address[MAX_ADDR];
    char password[64];
    double balance;
    int active; /* 1 = active, 0 = deleted */
} Account;

/* Utility: current timestamp string */
static void timestamp(char *buf, size_t n) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, n, "%Y-%m-%d %H:%M:%S", tm);
}

/* Append transaction to account-specific log */
static void log_transaction(int acc_no, const char *entry) {
    char fname[64];
    snprintf(fname, sizeof(fname), "trans_%d.txt", acc_no);
    FILE *f = fopen(fname, "a");
    if (!f) return;
    char ts[64]; timestamp(ts, sizeof(ts));
    fprintf(f, "[%s] %s\n", ts, entry);
    fclose(f);
}

/* Generate next account number */
static int next_account_number() {
    FILE *f = fopen(ACC_FILE, "rb");
    int max = 100000; /* base */
    if (!f) return max + 1;
    Account a;
    while (fread(&a, sizeof(a), 1, f) == 1) {
        if (a.active && a.acc_no > max) max = a.acc_no;
    }
    fclose(f);
    return max + 1;
}

/* Save account to file (append) */
static int save_account(const Account *a) {
    FILE *f = fopen(ACC_FILE, "ab");
    if (!f) return 0;
    fwrite(a, sizeof(*a), 1, f);
    fclose(f);
    return 1;
}

/* Find account by number, returns 1 and fills out if found */
static int find_account(int acc_no, Account *out) {
    FILE *f = fopen(ACC_FILE, "rb");
    if (!f) return 0;
    Account a;
    while (fread(&a, sizeof(a), 1, f) == 1) {
        if (a.active && a.acc_no == acc_no) {
            if (out) *out = a;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

/* Update account record by rewriting file */
static int update_account(const Account *updated) {
    FILE *f = fopen(ACC_FILE, "rb");
    if (!f) return 0;
    FILE *tmp = fopen(".tmp_acc.dat", "wb");
    if (!tmp) { fclose(f); return 0; }
    Account a;
    int replaced = 0;
    while (fread(&a, sizeof(a), 1, f) == 1) {
        if (a.acc_no == updated->acc_no) {
            fwrite(updated, sizeof(*updated), 1, tmp);
            replaced = 1;
        } else {
            fwrite(&a, sizeof(a), 1, tmp);
        }
    }
    fclose(f); fclose(tmp);
    if (!replaced) { remove(".tmp_acc.dat"); return 0; }
    remove(ACC_FILE);
    rename(".tmp_acc.dat", ACC_FILE);
    return 1;
}

/* Delete account: mark inactive and remove trans file */
static int delete_account(int acc_no) {
    Account a;
    if (!find_account(acc_no, &a)) return 0;
    a.active = 0;
    if (!update_account(&a)) return 0;
    char fname[64]; snprintf(fname, sizeof(fname), "trans_%d.txt", acc_no);
    remove(fname);
    return 1;
}

/* UI helpers */
static void center_puts(WINDOW *w, int y, const char *s) {
    int x = (getmaxx(w) - (int)strlen(s)) / 2;
    mvwprintw(w, y, x, "%s", s);
}

/* Input with label, supports masked mode */
static void input_field(WINDOW *w, int y, int x, const char *label, char *buf, int n, int masked) {
    mvwprintw(w, y, x, "%s: ", label);
    wrefresh(w);
    echo();
    if (masked) {
        noecho();
        int i = 0; int ch;
        while ((ch = wgetch(w)) != '\n' && ch != '\r') {
            if (ch == KEY_BACKSPACE || ch == 127) {
                if (i > 0) { i--; mvwdelch(w, y, x + (int)strlen(label) + 2 + i); }
            } else if (i < n-1) {
                buf[i++] = ch; mvwaddch(w, y, x + (int)strlen(label) + 2 + i -1, '*');
            }
        }
        buf[i] = '\0';
        echo();
    } else {
        wgetnstr(w, buf, n-1);
    }
}

/* Create new account flow */
static void ui_create_account(WINDOW *w) {
    werase(w); box(w,0,0);
    center_puts(w,1,"Create New Account");
    Account a; memset(&a,0,sizeof(a)); a.acc_no = next_account_number(); a.active = 1; a.balance = 0.0;
    char buf[256];
    mvwprintw(w,3,2,"Account Number: %d", a.acc_no);
    input_field(w,4,2,"Full Name", a.name, MAX_NAME, 0);
    input_field(w,5,2,"Gender", a.gender, 8, 0);
    input_field(w,6,2,"DOB (YYYY-MM-DD)", a.dob, 16, 0);
    input_field(w,7,2,"Email", a.email, MAX_EMAIL, 0);
    input_field(w,8,2,"Address", a.address, MAX_ADDR, 0);
    input_field(w,9,2,"Password", a.password, 64, 1);
    a.balance = 0.0;
    if (save_account(&a)) {
        snprintf(buf, sizeof(buf), "Account created: %d | Name: %s", a.acc_no, a.name);
        log_transaction(a.acc_no, "Account created");
        mvwprintw(w,11,2,"Success: %s", buf);
    } else {
        mvwprintw(w,11,2,"Failed to create account.");
    }
    mvwprintw(w,13,2,"Press any key to continue...");
    wrefresh(w); wgetch(w);
}

/* Simple account menu after auth */
static void ui_account_menu(WINDOW *w, Account *a) {
    int ch;
    while (1) {
        werase(w); box(w,0,0);
        char title[128]; snprintf(title, sizeof(title), "Account: %d - %s", a->acc_no, a->name);
        center_puts(w,1,title);
        mvwprintw(w,3,2,"Balance: %.2f", a->balance);
        mvwprintw(w,5,2,"1) Deposit");
        mvwprintw(w,6,2,"2) Withdraw");
        mvwprintw(w,7,2,"3) Transaction History");
        mvwprintw(w,8,2,"4) Delete Account");
        mvwprintw(w,9,2,"5) Logout");
        mvwprintw(w,11,2,"Choose: "); wrefresh(w);
        ch = wgetch(w);
        if (ch == '1') {
            char amt_s[64]; double amt = 0;
            input_field(w,13,2,"Amount", amt_s, sizeof(amt_s), 0);
            amt = atof(amt_s);
            if (amt > 0) {
                a->balance += amt;
                update_account(a);
                char e[128]; snprintf(e,sizeof(e),"Deposit: +%.2f | New Balance: %.2f", amt, a->balance);
                log_transaction(a->acc_no, e);
                mvwprintw(w,15,2,"Deposit successful.");
            } else mvwprintw(w,15,2,"Invalid amount.");
            mvwprintw(w,17,2,"Press any key..."); wrefresh(w); wgetch(w);
        } else if (ch == '2') {
            char amt_s[64]; double amt = 0;
            input_field(w,13,2,"Amount", amt_s, sizeof(amt_s), 0);
            amt = atof(amt_s);
            if (amt > 0 && amt <= a->balance) {
                a->balance -= amt;
                update_account(a);
                char e[128]; snprintf(e,sizeof(e),"Withdraw: -%.2f | New Balance: %.2f", amt, a->balance);
                log_transaction(a->acc_no, e);
                mvwprintw(w,15,2,"Withdrawal successful.");
            } else mvwprintw(w,15,2,"Insufficient funds or invalid amount.");
            mvwprintw(w,17,2,"Press any key..."); wrefresh(w); wgetch(w);
        } else if (ch == '3') {
            werase(w); box(w,0,0);
            center_puts(w,1,"Transaction History");
            char fname[64]; snprintf(fname,sizeof(fname),"trans_%d.txt", a->acc_no);
            FILE *f = fopen(fname, "r");
            int y = 3;
            if (!f) mvwprintw(w,y,2,"No transactions found.");
            else {
                char line[256];
                while (fgets(line, sizeof(line), f) && y < getmaxy(w)-2) {
                    mvwprintw(w,y++,2,"%s", line);
                }
                fclose(f);
            }
            mvwprintw(w,getmaxy(w)-2,2,"Press any key to return..."); wrefresh(w); wgetch(w);
        } else if (ch == '4') {
            mvwprintw(w,13,2,"Confirm delete? (y/n): "); wrefresh(w);
            int c = wgetch(w);
            if (c == 'y' || c == 'Y') {
                if (delete_account(a->acc_no)) {
                    mvwprintw(w,15,2,"Account deleted permanently."); wrefresh(w); wgetch(w);
                    break;
                } else {
                    mvwprintw(w,15,2,"Failed to delete account."); wrefresh(w); wgetch(w);
                }
            }
        } else if (ch == '5') break;
    }
}

/* Authentication flow */
static void ui_login(WINDOW *w) {
    werase(w); box(w,0,0);
    center_puts(w,1,"Login");
    char acc_s[32]; char pass[64];
    input_field(w,4,2,"Account Number", acc_s, sizeof(acc_s), 0);
    input_field(w,5,2,"Password", pass, sizeof(pass), 1);
    int acc_no = atoi(acc_s);
    Account a;
    if (!find_account(acc_no, &a)) {
        mvwprintw(w,7,2,"Account not found."); mvwprintw(w,9,2,"Press any key..."); wrefresh(w); wgetch(w); return;
    }
    if (strcmp(a.password, pass) != 0) {
        mvwprintw(w,7,2,"Invalid password."); mvwprintw(w,9,2,"Press any key..."); wrefresh(w); wgetch(w); return;
    }
    ui_account_menu(w, &a);
}

/* Main menu */
int main(void) {
    initscr(); cbreak(); noecho(); keypad(stdscr, TRUE);
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);
        init_pair(2, COLOR_RED, COLOR_BLUE);
    }
    bkgd(COLOR_PAIR(1));
    refresh();
    int h=24, w=80, y=(LINES-h)/2, x=(COLS-w)/2;
    WINDOW *win = newwin(h, w, y, x);
    wbkgd(win, COLOR_PAIR(1));
    int ch;
    while (1) {
        werase(win); box(win,0,0);
        center_puts(win,1,"Welcome to BlueBank™");
        mvwprintw(win,3,2,"1) Create Account");
        mvwprintw(win,4,2,"2) Login");
        mvwprintw(win,5,2,"3) Exit");
        mvwprintw(win,7,2,"Choose: "); wrefresh(win);
        ch = wgetch(win);
        if (ch == '1') ui_create_account(win);
        else if (ch == '2') ui_login(win);
        else if (ch == '3' || ch == 'q') break;
    }
    delwin(win); endwin();
    return 0;
}
