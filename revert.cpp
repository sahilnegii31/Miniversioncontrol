```cpp
#include "revert.h"
#include "branch.h"
#include <iostream>
#include <fstream>
#include <dirent.h>

using namespace std;

// Restore files from a selected commit snapshot
bool restoreSnapshot(string commitId, string username) {
    string snapshotDir = "data/objects/" + commitId;
    string workingDir  = "data/working/" + username;

    // Create required directories
    makeDir("data");
    makeDir("data/working");
    makeDir(workingDir);

    // Open the snapshot directory
    DIR* dir = opendir(snapshotDir.c_str());
    if (dir == nullptr) {
        cout << "Snapshot directory not found: " << snapshotDir << "\n";
        return false;
    }

    bool anyFileCopied = false;
    struct dirent* entry;

    // Copy each file from the snapshot to the working directory
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

// Checkout a specific commit by restoring its snapshot
bool checkoutCommitDirect(CommitNode* head, string commitId, string currentUser) {
    if (head == nullptr) return false;

    // Search for the requested commit
    CommitNode* target = findCommit(head, commitId);
    if (target == nullptr) return false;

    return restoreSnapshot(commitId, currentUser);
}

// Remove the latest commit from the linked list
CommitNode* undoLastCommitDirect(CommitNode* head, string currentUser) {
    if (head == nullptr) return nullptr;

    // If only one commit exists, delete the entire list
    if (head->next == nullptr) {
        delete head;
        saveCommits(nullptr);
        return nullptr;
    }

    // Traverse the list to find the last and second-last nodes
    CommitNode* prev = head;
    CommitNode* curr = head->next;

    while (curr->next != nullptr) {
        prev = curr;
        curr = curr->next;
    }

    // Store the parent commit ID before deleting the last node
    string parentId = curr->parentId;

    delete curr;
    prev->next = nullptr;

    
    if (parentId != "NONE") {
        restoreSnapshot(parentId, currentUser);
    }

    // Save the updated commit history
    saveCommits(head);
    return head;
}

// Revert the commit history to a selected commit
CommitNode* revertToCommitDirect(CommitNode* head, string targetId, string currentUser) {
    if (head == nullptr) return nullptr;

    // Find the target commit in the linked list
    CommitNode* target = findCommit(head, targetId);
    if (target == nullptr) return head;

    // Delete all commits that come after the target commit
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

// Checkout a commit after asking the user for confirmation
bool checkoutCommit(CommitNode* head, string commitId, string currentUser) {
    CommitNode* target = findCommit(head, commitId);

    if (target == nullptr) {
        cout << "Commit ID not found: " << commitId << "\n";
        return false;
    }

    // Display commit details
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

    // Restore the selected commit's files
    bool success = checkoutCommitDirect(head, commitId, currentUser);

    if (success) {
        cout << "Successfully moved to commit: " << commitId << "\n";
    } else {
        cout << "Failed to restore files from commit " << commitId << ".\n";
    }

    return success;
}

// Undo the last commit after user confirmation
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

    // Delete the latest commit and update the list
    head = undoLastCommitDirect(head, currentUser);

    cout << "Undo completed successfully.\n";
    return head;
}

// Revert to a selected commit after confirmation
CommitNode* revertToCommit(CommitNode* head, string targetId, string currentUser) {
    if (head == nullptr) {
        cout << "No commit history found.\n";
        return nullptr;
    }

    // Verify that the target commit exists
    CommitNode* target = findCommit(head, targetId);

    if (target == nullptr) {
        cout << "Commit ID not found: " << targetId << "\n";
        return head;
    }

    cout << "Reverting to commit: " << targetId
         << " (subsequent commits will be deleted)\n";
    cout << "Confirm? (y/n): ";

    char ch;
    cin >> ch;

    if (ch != 'y' && ch != 'Y') {
        cout << "Revert cancelled.\n";
        return head;
    }

    // Delete later commits and restore the selected snapshot
    head = revertToCommitDirect(head, targetId, currentUser);

    cout << "Revert complete! HEAD is now: " << targetId << "\n";
    return head;
}
```
