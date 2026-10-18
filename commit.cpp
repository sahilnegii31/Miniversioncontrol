#include "commit.h"
#include "revert.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <cstring>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
using namespace std;

string getCurrentTime() {
    time_t now = time(nullptr);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}

string generateCommitId(CommitNode* head) {
    int maxNum = 0;

    CommitNode* curr = head;
    while (curr != nullptr) {
        string idStr = curr->commitId.substr(1);
        int num = 0;
        for (int i = 0; i < (int)idStr.size(); i++) {
            num = num * 10 + (idStr[i] - '0');
        }
        if (num > maxNum) maxNum = num;
        curr = curr->next;
    }

    maxNum++;

    char buf[10];
    snprintf(buf, sizeof(buf), "C%03d", maxNum);
    return string(buf);
}

CommitNode* addCommit(CommitNode* head, string author, string message, string branch) {
    CommitNode* newNode = new CommitNode();
    newNode->commitId  = generateCommitId(head);
    newNode->author    = author;
    newNode->message   = message;
    newNode->branch    = branch;
    newNode->timestamp = getCurrentTime();

    if (head == nullptr) {
        newNode->parentId = "NONE";
    } else {
        CommitNode* curr = head;
        while (curr->next != nullptr) curr = curr->next;
        newNode->parentId = curr->commitId;
    }

    if (head == nullptr) {
        return newNode;
    }

    CommitNode* curr = head;
    while (curr->next != nullptr) curr = curr->next;
    curr->next = newNode;
    return head;
}

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

        for (int i = 0; i <= (int)line.size(); i++) {
            if (i == (int)line.size() || line[i] == '|') {
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

CommitNode* findCommit(CommitNode* head, string commitId) {
    CommitNode* curr = head;
    while (curr != nullptr) {
        if (curr->commitId == commitId) return curr;
        curr = curr->next;
    }
    return nullptr;
}

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
        string currentFilePath = currentDir + "/" + fname;
        string currentContent = "";
        ifstream curFile(currentFilePath.c_str());
        if (curFile.is_open()) {
            string line;
            while (getline(curFile, line)) {
                currentContent += line + "\n";
            }
            curFile.close();
        }

        cout << "\nFile: " << fname << "\n";

        if (curr->parentId == "NONE") {
            cout << "Status: [INITIAL COMMIT] Newly created file.\n";
            cout << "Content:\n";
            cout << "----------------------------------------\n";
            cout << currentContent;
            cout << "----------------------------------------\n";
        } else {
            string parentFilePath = "data/objects/" + curr->parentId + "/" + fname;
            ifstream parFile(parentFilePath.c_str());

            if (!parFile.is_open()) {
                cout << "Status: [ADDED] File was added in this commit.\n";
                cout << "Content:\n";
                cout << "----------------------------------------\n";
                cout << currentContent;
                cout << "----------------------------------------\n";
            } else {
                string parentContent = "";
                string line;
                while (getline(parFile, line)) {
                    parentContent += line + "\n";
                }
                parFile.close();

                if (currentContent == parentContent) {
                    cout << "Status: [UNCHANGED] Identical to parent commit (" << curr->parentId << ").\n";
                    cout << "Content:\n";
                    cout << "----------------------------------------\n";
                    cout << currentContent;
                    cout << "----------------------------------------\n";
                } else {
                    cout << "Status: [MODIFIED] File changed from parent commit (" << curr->parentId << ").\n";
                    cout << "\nPrevious Content (" << curr->parentId << "):\n";
                    cout << "----------------------------------------\n";
                    cout << parentContent;
                    cout << "----------------------------------------\n";
                    cout << "Current Content (" << commitId << "):\n";
                    cout << "----------------------------------------\n";
                    cout << currentContent;
                    cout << "----------------------------------------\n";
                }
            }
        }
    }
    closedir(dir);

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
                    cout << "Status: [DELETED] Existed in parent (" << curr->parentId << ") but removed in this commit.\n";
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

void freeCommits(CommitNode* head) {
    while (head != nullptr) {
        CommitNode* temp = head;
        head = head->next;
        delete temp;
    }
}
