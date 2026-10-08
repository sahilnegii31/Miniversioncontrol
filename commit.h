#ifndef COMMIT_H
#define COMMIT_H

#include <iostream>
#include <string>

using namespace std;

//MODULE 1

class Commit {
private:
    string commitId;        
    string message;         
    string fileContent;     
    string author;          
    string timestamp;       
    Commit* prev;           // Pointer to previous commit (backward linked list)

public:

    Commit(string id, string msg, string files, string auth, string time);


    string getId() const;
    string getMessage() const;
    string getFileContent() const;
    string getAuthor() const;
    string getTimestamp() const;
    Commit* getPrev() const;

    void setPrev(Commit* previousCommit);


    void display() const;
};

// Helper functions for Commit generation
string generateCommitId();
string getCurrentTimestamp();

#endif
