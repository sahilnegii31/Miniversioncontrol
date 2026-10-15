#include "revert.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
using namespace std;

static bool copyFile(string src, string dest) {
    ifstream in(src.c_str());
    if (!in.is_open()) {
        cout << "Source file not found: " << src << "\n";
        return false;
    }

    ofstream out(dest.c_str());
    if (!out.is_open()) {
        cout << "Destination file could not be opened: " << dest << "\n";
        return false;
    }

    string line;
    while (getline(in, line)) {
        out << line << "\n";
    }

    in.close();
    out.close();
    return true;
}

static void makeDir(string path) {
    mkdir(path.c_str(), 0755);
}

bool restoreSnapshot(string commitId, string username) {
    string snapshotDir = "data/objects/" + commitId;
    string workingDir  = "data/working/" + username;

    makeDir("data");
    makeDir("data/working");
    makeDir(workingDir);

    DIR* dir = opendir(snapshotDir.c_str());
    if (dir == nullptr) {
        cout << "Snapshot directory not found: " << snapshotDir << "\n";
        return false;
    }

    bool anyFileCopied = false;
    struct dirent* entry;

    while ((entry = readdir(dir)) != nullptr) {
        string fname = entry->d_name;

        if (fname == "." || fname == "..") continue;

        string src  = snapshotDir + "/" + fname;
        string dest = workingDir  + "/" + fname;

        if (copyFile(src, dest)) {
            cout << "Restored: " << fname << "\n";
            anyFileCopied = true;
        }
    }

    closedir(dir);

    if (!anyFileCopied) {
        cout << "No files found in snapshot.\n";
    }

    return anyFileCopied;
}

bool checkoutCommit(CommitNode* head, string commitId, string currentUser) {
    CommitNode* target = findCommit(head, commitId);
    if (target == nullptr) {
        cout << "Commit ID not found: " << commitId << "\n";
        return false;
    }

    cout << "Checking out commit: " << commitId << "\n";
    cout << "Author   : " << target->author << "\n";
    cout << "Message  : " << target->message << "\n";
    cout << "Timestamp: " << target->timestamp << "\n";
    cout << "Warning: This will overwrite files in your working directory with this snapshot.\n";
    cout << "Confirm? (y/n): ";
    char ch;
    cin >> ch;
    if (ch != 'y' && ch != 'Y') {
        cout << "Checkout cancelled.\n";
        return false;
    }

    bool success = restoreSnapshot(commitId, currentUser);
    if (success) {
        cout << "Successfully moved to commit: " << commitId << "\n";
        cout << "Your working directory now contains files from " << commitId << ".\n";
    } else {
        cout << "Failed to restore files from commit " << commitId << ".\n";
    }
    return success;
}

CommitNode* undoLastCommit(CommitNode* head, string currentUser) {
    if (head == nullptr) {
        cout << "No commits available to undo.\n";
        return nullptr;
    }

    if (head->next == nullptr) {
        cout << "Only one commit exists. Undoing it will clear the history.\n";
        cout << "Confirm? (y/n): ";
        char ch;
        cin >> ch;
        if (ch != 'y' && ch != 'Y') {
            cout << "Undo cancelled.\n";
            return head;
        }

        string undoneId = head->commitId;
        delete head;
        saveCommits(nullptr);
        cout << "Commit " << undoneId << " undone. History is now empty.\n";
        return nullptr;
    }

    CommitNode* prev = head;
    CommitNode* curr = head->next;

    while (curr->next != nullptr) {
        prev = curr;
        curr = curr->next;
    }

    string undoneId = curr->commitId;
    string parentId = curr->parentId;

    cout << "Commit to undo: " << undoneId << "\n";
    cout << "Reverting to: " << parentId << "\n";
    cout << "Confirm? (y/n): ";
    char ch;
    cin >> ch;

    if (ch != 'y' && ch != 'Y') {
        cout << "Undo cancelled.\n";
        return head;
    }

    delete curr;
    prev->next = nullptr;

    if (parentId != "NONE") {
        cout << "Restoring parent commit...\n";
        restoreSnapshot(parentId, currentUser);
    }

    saveCommits(head);
    cout << "Commit " << undoneId << " undone successfully!\n";

    return head;
}

CommitNode* revertToCommit(CommitNode* head, string targetId, string currentUser) {
    if (head == nullptr) {
        cout << "No commit history found.\n";
        return nullptr;
    }

    CommitNode* target = findCommit(head, targetId);
    if (target == nullptr) {
        cout << "Commit ID not found: " << targetId << "\n";
        return head;
    }

    cout << "Reverting to commit: " << targetId << "\n";
    cout << "All subsequent commits will be deleted!\n";
    cout << "Confirm? (y/n): ";
    char ch;
    cin >> ch;

    if (ch != 'y' && ch != 'Y') {
        cout << "Revert cancelled.\n";
        return head;
    }

    CommitNode* curr = target->next;
    while (curr != nullptr) {
        CommitNode* temp = curr;
        curr = curr->next;
        delete temp;
    }
    target->next = nullptr;

    cout << "Restoring snapshot for: " << targetId << "\n";
    restoreSnapshot(targetId, currentUser);

    saveCommits(head);
    cout << "Revert complete! HEAD is now: " << targetId << "\n";

    return head;
}
