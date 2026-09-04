#include "commit.h"
#include<iostream>
#include <string>
// static int counter = 0 ;
using namespace std;

commit* head = nullptr ; // yaha pe extern define hai 

string generateId(){
    static int counter = 0 ; 
    string commitId = "C" + to_string(counter+1);
    cout << commitId;
    counter++ ;
    return commitId;
}

string generateCurrentTime(){
    return "testing";
}

// working of makecommit-
//takes content and the message then add it to the new commit node and connect the new node to the head and then move the head to the new node
void makeCommit(string content , string message){
    commit* newcommit = new commit(generateId(), message , content , generateCurrentTime());
    // newcommit->commitId = generateId();
    // newcommit->message = message;
    // newcommit->filecontent = content;
    // newcommit->timestamp = generateCurrentTime();
    // newcommit->prev = head ; 
    head = newcommit ; 
}

int main(){
    generateId();
    generateId();
    generateId();
    generateId();
    return 0;
}