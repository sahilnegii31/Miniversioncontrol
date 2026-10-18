#ifndef STAGING_H
#define STAGING_H

#include "stack.h"
#include "commit.h"
#include <string>
#include <vector>
using namespace std;

struct StagedFile {
    string filename;
    string content;

    StagedFile() : filename(""), content("") {}
    StagedFile(string f, string c) : filename(f), content(c) {}
};

class StagingArea {
private:
    Stack<StagedFile> stagingStack;

public:
    void stageFile(string filename, string content);
    void unstageLastFile();
    void viewStaging();
    CommitNode* commitAll(CommitNode* head, string author, string branch);
    CommitNode* commitAllWithMessage(CommitNode* head, string author, string branch, const string& commitMsg);
    vector<StagedFile> getStagedFiles();
    void clear();
    bool isEmpty();
    int getCount();
};

#endif
