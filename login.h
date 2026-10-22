#ifndef LOGIN_H
#define LOGIN_H

#include <string>
using namespace std;

#define MAX_USERS 100

// User class representing a registered user account
class User {
public:
    char username[50];
    char password[50];

    User();
    User(const string& uname, const string& pass);
};

// UserManager class encapsulating user operations
class UserManager {
public:
    User users[MAX_USERS];
    int count;

    UserManager();

    int load();
    void save();
    bool registerAccount();
    string login();
    string runMenu();
};

void ensureDataFolder();
int loadUsers(User users[], int maxUsers);
void saveUsers(User users[], int count);
bool registerUser(User users[], int &count);
string loginUser(User users[], int count);
string runLoginMenu();

bool loginUserDirect(User users[], int count, const string& uname, const string& pass);
bool registerUserDirect(User users[], int &count, const string& uname, const string& pass, string& errMsg);

#endif
