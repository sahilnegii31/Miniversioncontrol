#include "staging.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
using namespace std;

static void makeDir(string path) {
    mkdir(path.c_str(), 0755);
}

void StagingArea::stageFile(string filename, string content) {
    Stack<StagedFile> tempStack;
    bool found = false;

    while (!stagingStack.isEmpty()) {
        StagedFile f = stagingStack.pop();
        if (f.filename == filename) {
            f.content = content;
            found = true;
        }
        tempStack.push(f);
    }

    while (!tempStack.isEmpty()) {
        stagingStack.push(tempStack.pop());
    }

    if (!found) {
        stagingStack.push(StagedFile(filename, content));
        cout << "File staged: " << filename << "\n";
    } else {
        cout << "File updated in staging: " << filename << "\n";
    }
}

void StagingArea::unstageLastFile() {
    if (stagingStack.isEmpty()) {
        cout << "Staging area is empty. Nothing to remove.\n";
        return;
    }

    StagedFile removed = stagingStack.pop();
    cout << "Unstaged: " << removed.filename << "\n";
}

void StagingArea::viewStaging() {
    if (stagingStack.isEmpty()) {
        cout << "No files in staging area.\n";
        return;
    }

    cout << "\n===== Staged Files =====\n";

    Stack<StagedFile> tempStack;
    int i = 1;

    while (!stagingStack.isEmpty()) {
        StagedFile f = stagingStack.pop();
        cout << i++ << ". " << f.filename << "\n";
        cout << "   Content: " << f.content << "\n";
        tempStack.push(f);
    }

    while (!tempStack.isEmpty()) {
        stagingStack.push(tempStack.pop());
    }
}

CommitNode* StagingArea::commitAll(CommitNode* head, string author, string branch) {
    if (stagingStack.isEmpty()) {
        cout << "No staged files. Please stage files first using 'add'.\n";
        return head;
    }

    string commitMsg;
    cout << "Enter commit message: ";
    cin.ignore();
    getline(cin, commitMsg);

    head = addCommit(head, author, commitMsg, branch);

    CommitNode* curr = head;
    while (curr->next != nullptr) curr = curr->next;
    string newId = curr->commitId;

    makeDir("data");
    makeDir("data/objects");
    string snapshotDir = "data/objects/" + newId;
    makeDir(snapshotDir);

    int fileCount = 0;
    while (!stagingStack.isEmpty()) {
        StagedFile f = stagingStack.pop();
        string filePath = snapshotDir + "/" + f.filename;

        ofstream out(filePath.c_str());
        if (out.is_open()) {
            out << f.content;
            out.close();
            cout << "  Saved: " << f.filename << " -> " << filePath << "\n";
            fileCount++;
        } else {
            cout << "  Warning: Could not save " << f.filename << ".\n";
        }
    }

    saveCommits(head);

    cout << "\nCommit successful!\n";
    cout << "Commit ID : " << newId << "\n";
    cout << "Files saved: " << fileCount << "\n";

    return head;
}

bool StagingArea::isEmpty() {
    return stagingStack.isEmpty();
}

int StagingArea::getCount() {
    return stagingStack.size();
}
