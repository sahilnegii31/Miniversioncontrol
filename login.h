#ifndef LOGIN_H
#define LOGIN_H

#include <string>
using namespace std;

#define MAX_USERS 100

struct User {
    char username[50];
    char password[50];
};

void ensureDataFolder();
int loadUsers(User users[], int maxUsers);
void saveUsers(User users[], int count);
bool registerUser(User users[], int &count);
string loginUser(User users[], int count);
string runLoginMenu();

#endif
