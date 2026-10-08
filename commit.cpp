#include "commit.h"
#include <iostream>
#include <string>
#include <ctime>

using namespace std;

// Constructor: Initializes all attributes and sets prev pointer to nullptr
Commit::Commit(string id, string msg, string files, string auth, string time) {
    commitId = id;
    message = msg;
    fileContent = files;
    author = auth;
    timestamp = time;
    prev = nullptr;
}


string Commit::getId() const { 
    return commitId; 
}

string Commit::getMessage() const { 
    return message; 
}

string Commit::getFileContent() const { 
    return fileContent; 
}

string Commit::getAuthor() const { 
    return author; 
}

string Commit::getTimestamp() const { 
    return timestamp; 
}

Commit* Commit::getPrev() const { 
    return prev; 
}


void Commit::setPrev(Commit* previousCommit) {
    prev = previousCommit;
}

// Display commit details in a neat card format
void Commit::display() const {
    cout << "--------------------------------------------------" << endl;
    cout << "Commit ID : " << commitId << endl;
    cout << "Author    : " << author << endl;
    cout << "Date/Time : " << timestamp << endl;
    cout << "Files     : " << (fileContent.empty() ? "(None)" : fileContent) << endl;
    cout << "Message   : " << message << endl;
    cout << "--------------------------------------------------" << endl;
}

string generateCommitId() {
    static int counter = 1;
    string id = "C" + to_string(counter);
    counter++;
    return id;
}

// Generates real system timestamp formatted as readable text
string getCurrentTimestamp() {
    time_t now = time(0);
    char* dt = ctime(&now);
    string timeStr(dt);

    // ctime appends a newline at the end, let's remove it
    
    if (!timeStr.empty() && timeStr.back() == '\n') {
        timeStr.pop_back();
    }
    return timeStr;
}