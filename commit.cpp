#include "commit.h"
#include "revert.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <dirent.h>
#include <sys/stat.h>

using namespace std;

// Helper to read entire file content as string
static string readFileContent(string path) {
    ifstream f(path.c_str());
    if (!f.is_open()) return "";
    string content = "", line;
    while (getline(f, line)) {
        content += line + "\n";
    }
    return content;
}

// Get current date and time as a formatted string
string getCurrentTime() {
    time_t now = time(nullptr);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}

// Generate sequential commit ID (C001, C002, ...)
string generateCommitId(CommitNode* head) {
    int maxNum = 0;
    CommitNode* curr = head;

    while (curr != nullptr) {
        string idStr = curr->commitId.substr(1);
        int num = 0;
        for (char c : idStr) {
            num = num * 10 + (c - '0');
        }
        if (num > maxNum) maxNum = num;
        curr = curr->next;
    }

    maxNum++;
    char buf[10];
    snprintf(buf, sizeof(buf), "C%03d", maxNum);
    return string(buf);
}

// Append a new commit node to the end of the linked list
CommitNode* addCommit(CommitNode* head, string author, string message, string branch) {
    CommitNode* newNode = new CommitNode();
    newNode->commitId  = generateCommitId(head);
    newNode->author    = author;
    newNode->message   = message;
    newNode->branch    = branch;
    newNode->timestamp = getCurrentTime();

    if (head == nullptr) {
        newNode->parentId = "NONE";
        return newNode;
    }

    // Traverse to the last node to set parent and link
    CommitNode* curr = head;
    while (curr->next != nullptr) {
        curr = curr->next;
    }

    newNode->parentId = curr->commitId;
    curr->next = newNode;
    return head;
}

// Load commits from data/commits.txt into a linked list
CommitNode* loadCommits() {
    ifstream file("data/commits.txt");
    if (!file.is_open()) return nullptr;

    CommitNode* head = nullptr;
    CommitNode* tail = nullptr;
    string line;

    while (getline(file, line)) {
        if (line.empty()) continue;

        string parts[6];
        int partIndex = 0;
        string temp = "";

        for (size_t i = 0; i <= line.size(); i++) {
            if (i == line.size() || line[i] == '|') {
                if (partIndex < 6) parts[partIndex++] = temp;
                temp = "";
            } else {
                temp += line[i];
            }
        }

        if (partIndex < 6) continue;

        CommitNode* node = new CommitNode();
        node->commitId  = parts[0];
        node->author    = parts[1];
        node->message   = parts[2];
        node->timestamp = parts[3];
        node->branch    = parts[4];
        node->parentId  = parts[5];
        node->next      = nullptr;

        if (head == nullptr) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }

    file.close();
    return head;
}

// Save the commit linked list to data/commits.txt
void saveCommits(CommitNode* head) {
    mkdir("data", 0755);

    ofstream file("data/commits.txt");
    if (!file.is_open()) {
        cout << "Error: Could not save commits.txt.\n";
        return;
    }

    CommitNode* curr = head;
    while (curr != nullptr) {
        file << curr->commitId  << "|"
             << curr->author    << "|"
             << curr->message   << "|"
             << curr->timestamp << "|"
             << curr->branch    << "|"
             << curr->parentId  << "\n";
        curr = curr->next;
    }

    file.close();
}

// Print full commit history (git log)
void showHistory(CommitNode* head) {
    if (head == nullptr) {
        cout << "No commits found yet.\n";
        return;
    }

    cout << "\n===== Commit History =====\n";
    CommitNode* curr = head;
    while (curr != nullptr) {
        cout << "ID      : " << curr->commitId  << "\n";
        cout << "Author  : " << curr->author    << "\n";
        cout << "Message : " << curr->message   << "\n";
        cout << "Time    : " << curr->timestamp << "\n";
        cout << "Branch  : " << curr->branch    << "\n";
        cout << "Parent  : " << curr->parentId  << "\n";
        cout << "----------------------------\n";
        curr = curr->next;
    }
}

// Find a commit by its commit ID in the linked list
CommitNode* findCommit(CommitNode* head, string commitId) {
    CommitNode* curr = head;
    while (curr != nullptr) {
        if (curr->commitId == commitId) return curr;
        curr = curr->next;
    }
    return nullptr;
}

// Inspect changes introduced in a commit compared to its parent
void viewCommitChanges(CommitNode* head, string commitId, string currentUser) {
    CommitNode* curr = findCommit(head, commitId);
    if (curr == nullptr) {
        cout << "Commit ID not found: " << commitId << "\n";
        return;
    }

    cout << "\n========================================\n";
    cout << "Commit ID    : " << curr->commitId << "\n";
    cout << "Author       : " << curr->author << "\n";
    cout << "Message      : " << curr->message << "\n";
    cout << "Timestamp    : " << curr->timestamp << "\n";
    cout << "Branch       : " << curr->branch << "\n";
    cout << "Parent Commit: " << curr->parentId << "\n";
    cout << "========================================\n";

    string currentDir = "data/objects/" + commitId;
    DIR* dir = opendir(currentDir.c_str());
    if (dir == nullptr) {
        cout << "Snapshot directory not found for commit: " << commitId << "\n";
        return;
    }

    cout << "\n--- Files in Commit " << commitId << " ---\n";
    bool foundFiles = false;
    struct dirent* entry;

    while ((entry = readdir(dir)) != nullptr) {
        string fname = entry->d_name;
        if (fname == "." || fname == "..") continue;

        foundFiles = true;
        string currentContent = readFileContent(currentDir + "/" + fname);

        cout << "\nFile: " << fname << "\n";

        if (curr->parentId == "NONE") {
            cout << "Status: [INITIAL COMMIT] Newly created file.\n";
            cout << "Content:\n" << currentContent;
        } else {
            string parentPath = "data/objects/" + curr->parentId + "/" + fname;
            ifstream parTest(parentPath.c_str());

            if (!parTest.is_open()) {
                cout << "Status: [ADDED] File was added in this commit.\n";
                cout << "Content:\n" << currentContent;
            } else {
                parTest.close();
                string parentContent = readFileContent(parentPath);

                if (currentContent == parentContent) {
                    cout << "Status: [UNCHANGED] Identical to parent (" << curr->parentId << ").\n";
                } else {
                    cout << "Status: [MODIFIED] File changed from parent (" << curr->parentId << ").\n";
                    cout << "Previous Content:\n" << parentContent;
                    cout << "Current Content:\n"  << currentContent;
                }
            }
        }
    }
    closedir(dir);

    // Check for deleted files
    if (curr->parentId != "NONE") {
        string parentDir = "data/objects/" + curr->parentId;
        DIR* pDir = opendir(parentDir.c_str());
        if (pDir != nullptr) {
            struct dirent* pEntry;
            while ((pEntry = readdir(pDir)) != nullptr) {
                string pFname = pEntry->d_name;
                if (pFname == "." || pFname == "..") continue;

                string checkPath = currentDir + "/" + pFname;
                ifstream checkFile(checkPath.c_str());
                if (!checkFile.is_open()) {
                    cout << "\nFile: " << pFname << "\n";
                    cout << "Status: [DELETED] Removed in this commit.\n";
                } else {
                    checkFile.close();
                }
            }
            closedir(pDir);
        }
    }

    if (!foundFiles) {
        cout << "No files found in this commit snapshot.\n";
    }

    cout << "\nDo you want to checkout/move working directory to this commit? (y/n): ";
    char ch;
    cin >> ch;
    if (ch == 'y' || ch == 'Y') {
        restoreSnapshot(commitId, currentUser);
        cout << "Working directory updated to commit " << commitId << ".\n";
    }
}

// Free all nodes in the commit linked list
void freeCommits(CommitNode* head) {
    while (head != nullptr) {
        CommitNode* temp = head;
        head = head->next;
        delete temp;
    }
}
