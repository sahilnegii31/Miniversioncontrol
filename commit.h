#ifndef COMMIT_H
#define COMMIT_H

#include<iostream>
using namespace std;

class commit {
    public: 
        string commitId;
        string message ; 
        string filecontent;
        string author;
        string timestamp;
        commit* prev;
        commit(string id, string message , string filecontent, string timestamp ){
            commitId = id;
            this->message = message;
            this->filecontent = filecontent;
            this->timestamp = timestamp;
            this->author = "someone" ;

        }
};

extern commit* head; //Jaha hum abhi hai

string generateId();
string generateCurrentTime();
void makeCommit( string content , string message);
void showHistory();

#endif
