#include "revert.h"
#include "branch.h"
#include <iostream>
#include <fstream>
#include <dirent.h>

using namespace std;


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
        if (fname != "." && fname != "..") {
            string src  = snapshotDir + "/" + fname;
            string dest = workingDir  + "/" + fname;
            if (copyFile(src, dest)) {
                anyFileCopied = true;
            }
        }
    }
    closedir(dir);
    return anyFileCopied;
}


bool checkoutCommitDirect(CommitNode* head, string commitId, string currentUser) {
    if (head == nullptr) return false;
    CommitNode* target = findCommit(head, commitId);
    if (target == nullptr) return false;
    return restoreSnapshot(commitId, currentUser);
}


CommitNode* undoLastCommitDirect(CommitNode* head, string currentUser) {
    if (head == nullptr) return nullptr;

    
    if (head->next == nullptr) {
        delete head;
        saveCommits(nullptr);
        return nullptr;
    }

    
    CommitNode* prev = head;
    CommitNode* curr = head->next;

    while (curr->next != nullptr) {
        prev = curr;
        curr = curr->next;
    }

    string parentId = curr->parentId;
    delete curr;
    prev->next = nullptr;

    if (parentId != "NONE") {
        restoreSnapshot(parentId, currentUser);
    }

    saveCommits(head);
    return head;
}


CommitNode* revertToCommitDirect(CommitNode* head, string targetId, string currentUser) {
    if (head == nullptr) return nullptr;

    CommitNode* target = findCommit(head, targetId);
    if (target == nullptr) return head;

    
    CommitNode* curr = target->next;
    while (curr != nullptr) {
        CommitNode* temp = curr;
        curr = curr->next;
        delete temp;
    }
    target->next = nullptr;

    restoreSnapshot(targetId, currentUser);
    saveCommits(head);
    return head;
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
    cout << "Confirm? (y/n): ";
    char ch;
    cin >> ch;
    if (ch != 'y' && ch != 'Y') {
        cout << "Checkout cancelled.\n";
        return false;
    }

    bool success = checkoutCommitDirect(head, commitId, currentUser);
    if (success) {
        cout << "Successfully moved to commit: " << commitId << "\n";
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

    cout << "Confirm undo last commit? (y/n): ";
    char ch;
    cin >> ch;
    if (ch != 'y' && ch != 'Y') {
        cout << "Undo cancelled.\n";
        return head;
    }

    head = undoLastCommitDirect(head, currentUser);
    cout << "Undo completed successfully.\n";
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

    cout << "Reverting to commit: " << targetId << " (subsequent commits will be deleted)\n";
    cout << "Confirm? (y/n): ";
    char ch;
    cin >> ch;
    if (ch != 'y' && ch != 'Y') {
        cout << "Revert cancelled.\n";
        return head;
    }

    head = revertToCommitDirect(head, targetId, currentUser);
    cout << "Revert complete! HEAD is now: " << targetId << "\n";
    return head;
} 
