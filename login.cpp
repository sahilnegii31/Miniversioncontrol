#include "login.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <direct.h>
#include <sys/stat.h>
#include <cstdlib>

using namespace std;



void ensureDataFolder() {
    _mkdir("data");
}

int loadUsers(User users[], int maxUsers) {
    ensureDataFolder();

    ifstream file("data/users.txt");

    if (!file.is_open()) {
        return 0;
    }

    int count = 0;
    string line;

    while (getline(file, line) && count < maxUsers) {
        int colonPos = -1;

        for (int i = 0; i < (int)line.size(); i++) {
            if (line[i] == ':') {
                colonPos = i;
                break;
            }
        }

        if (colonPos == -1)
            continue;

        string uname = line.substr(0, colonPos);
        string pass = line.substr(colonPos + 1);

        strncpy(users[count].username, uname.c_str(), 49);
        strncpy(users[count].password, pass.c_str(), 49);

        users[count].username[49] = '\0';
        users[count].password[49] = '\0';

        count++;
    }

    file.close();
    return count;
}

void saveUsers(User users[], int count) {
    ensureDataFolder();

    ofstream file("data/users.txt");

    if (!file.is_open()) {
        cout << "Error: Could not save users.txt.\n";
        return;
    }

    for (int i = 0; i < count; i++) {
        file << users[i].username << ":"
             << users[i].password << "\n";
    }

    file.close();
}

bool registerUser(User users[], int &count) {
    if (count >= MAX_USERS) {
        cout << "User limit reached. Cannot register more users.\n";
        return false;
    }

    string uname, pass;

    cout << "Enter new username: ";
    cin >> uname;

    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, uname.c_str()) == 0) {
            cout << "This username already exists!\n";
            return false;
        }
    }

    cout << "Enter password: ";
    cin >> pass;

    strncpy(users[count].username, uname.c_str(), 49);
    strncpy(users[count].password, pass.c_str(), 49);

    users[count].username[49] = '\0';
    users[count].password[49] = '\0';

    count++;

    saveUsers(users, count);

    cout << "Account created successfully! You can now log in.\n";

    return true;
}

string loginUser(User users[], int count) {
    string uname, pass;

    cout << "Enter username: ";
    cin >> uname;

    cout << "Enter password: ";
    cin >> pass;

    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, uname.c_str()) == 0 &&
            strcmp(users[i].password, pass.c_str()) == 0) {

            cout << "Login successful! Welcome, "
                 << uname << "\n";

            return uname;
        }
    }

    cout << "Invalid username or password.\n";

    return "";
}

string runLoginMenu() {
    User users[MAX_USERS];

    int count = loadUsers(users, MAX_USERS);

    string loggedIn = "";

    while (loggedIn.empty()) {
        cout << "\n===== MiniVCS Login =====\n";
        cout << "1. Login\n";
        cout << "2. Register\n";
        cout << "3. Exit\n";
        cout << "Choice: ";

        int choice;
        cin >> choice;

        if (choice == 1) {
            loggedIn = loginUser(users, count);
        }
        else if (choice == 2) {
            registerUser(users, count);
        }
        else if (choice == 3) {
            exit(0);
        }
        else {
            cout << "Invalid choice. Please try again.\n";
        }
    }

    return loggedIn;
}