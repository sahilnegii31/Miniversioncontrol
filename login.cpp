#include<iostream>
#include<unordered_map>
using namespace std ;

static unordered_map<string , string>user ; 
extern string currentUser;  //yeh global rahega tki hme pta ho konsa user loggined hai 
void registerUser(){
    string username , password ;
    cout << "Enter Username : " << " ";
    cin >> username ; 
    cout << "Enter Password : " << " ";
    cin >> password ; 
    user.insert({username , password});
}
string currentUser;
bool login(){
    string username , password ; 
    cout << "Enter Username : " << " ";
    cin >> username ;
    cout << "Enter Password : " << " ";
    cin >> password ; 
    if(user.find(username) != user.end() && user[username] == password ){
        currentUser = username;
        cout << "Login Successful!! \n " << endl ;
        return true ;
    }
    cout << "Invalid Credentials!! \n " << endl ; 
    return false;
}