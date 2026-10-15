#ifndef STAGING_H
#define STAGING_H

#include "stack.h"
#include "commit.h"
#include <string>
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
    bool isEmpty();
    int getCount();
};

#endif
