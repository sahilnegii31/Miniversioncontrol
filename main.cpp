#include<iostream>
#include "commit.h"
#include "login.cpp"
using namespace std;


int main(){
cout << "MINI VERSION CONTROL SYSTEM" << endl ; 
int ch ;
bool islogin = false;
do{
    cout << "1. Commit \n 2. Login \n 3.Register \n Enter Your Choice : " << endl;
    cin >> ch;
    switch(ch){
        case 1:
        break;
        case 2:
        islogin = login();
        break;
        case 3:
        registerUser();
        break;
        case 4:
        break;
        break;
        default:
        cout << "Invalid Choice!!" << endl;
        break;
    }
}while(1);
return 0;
}
