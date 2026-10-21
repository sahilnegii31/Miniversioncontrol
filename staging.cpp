#include "staging.h"
#include "branch.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

using namespace std;

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
    }
}

void StagingArea::unstageLastFile() {
    if (stagingStack.isEmpty()) {
        cout << "Staging area is empty.\n";
        return;
    }
    StagedFile removed = stagingStack.pop();
    cout << "Unstaged: " << removed.filename << "\n";
}

void StagingArea::viewStaging() {
    if (stagingStack.isEmpty()) {
        cout << "Staging area is empty.\n";
        return;
    }

    cout << "\n===== Staged Files =====\n";
    Stack<StagedFile> tempStack;
    int i = 1;

    while (!stagingStack.isEmpty()) {
        StagedFile f = stagingStack.pop();
        cout << i++ << ". " << f.filename << " (" << f.content.size() << " bytes)\n";
        tempStack.push(f);
    }

    while (!tempStack.isEmpty()) {
        stagingStack.push(tempStack.pop());
    }
}

CommitNode* StagingArea::commitAllWithMessage(CommitNode* head, string author, string branch, const string& commitMsg) {
    if (stagingStack.isEmpty()) {
        return head;
    }

    syncWorkingWithBranch(author, branch);

    head = addCommit(head, author, commitMsg, branch);

    CommitNode* curr = head;
    while (curr->next != nullptr) curr = curr->next;
    string newId = curr->commitId;

    makeDir("data");
    makeDir("data/objects");
    string snapshotDir = "data/objects/" + newId;
    makeDir(snapshotDir);

    string branchDir = "data/branches/" + branch;
    if (dirExists(branchDir)) {
        copyDirectory(branchDir, snapshotDir);
    }

    int fileCount = 0;
    while (!stagingStack.isEmpty()) {
        StagedFile f = stagingStack.pop();
        string filePath = snapshotDir + "/" + f.filename;

        ofstream out(filePath.c_str());
        if (out.is_open()) {
            out << f.content;
            out.close();
            fileCount++;
        }
    }

    saveCommits(head);

    syncBranchWithCommit(branch, newId);

    string workDir = "data/working/" + author;
    if (dirExists(workDir) && getUserCurrentBranch(author) == branch) {
        copyDirectory(snapshotDir, workDir);
    }

    cout << "Commit " << newId << " created on branch '" << branch << "' (" << fileCount << " files modified).\n";
    return head;
}

CommitNode* StagingArea::commitAll(CommitNode* head, string author, string branch) {
    if (stagingStack.isEmpty()) {
        cout << "No staged files. Stage files first using 'git add'.\n";
        return head;
    }

    string commitMsg;
    cout << "Enter commit message: ";
    cin.ignore();
    getline(cin, commitMsg);

    return commitAllWithMessage(head, author, branch, commitMsg);
}

vector<StagedFile> StagingArea::getStagedFiles() {
    vector<StagedFile> result;
    Stack<StagedFile> tempStack;
    while (!stagingStack.isEmpty()) {
        StagedFile f = stagingStack.pop();
        result.push_back(f);
        tempStack.push(f);
    }
    while (!tempStack.isEmpty()) {
        stagingStack.push(tempStack.pop());
    }
    return result;
}

void StagingArea::clear() {
    while (!stagingStack.isEmpty()) {
        stagingStack.pop();
    }
}

bool StagingArea::isEmpty() {
    return stagingStack.isEmpty();
}

int StagingArea::getCount() {
    return stagingStack.size();
}
