#ifndef COMMIT_H
#define COMMIT_H

#include <string>
using namespace std;

// CommitNode class representing a node in the commit linked list
class CommitNode {
public:
    string commitId;
    string author;
    string message;
    string timestamp;
    string branch;
    string parentId;

    CommitNode* next;

    CommitNode() : next(nullptr) {}

    CommitNode(string id, string auth, string msg, string time, string br, string parent = "NONE")
        : commitId(id), author(auth), message(msg), timestamp(time), branch(br), parentId(parent), next(nullptr) {}
};

// CommitHistory class encapsulating commit history operations
class CommitHistory {
public:
    CommitNode* head;

    CommitHistory();
    ~CommitHistory();

    void add(string author, string message, string branch);
    void load();
    void save();
    void show();
    CommitNode* find(string id);
    void freeHistory();
};

string getCurrentTime();
string generateCommitId(CommitNode* head);
CommitNode* addCommit(CommitNode* head, string author, string message, string branch);
CommitNode* loadCommits();
void saveCommits(CommitNode* head);
void showHistory(CommitNode* head);
CommitNode* findCommit(CommitNode* head, string commitId);
void viewCommitChanges(CommitNode* head, string commitId, string currentUser);
void freeCommits(CommitNode* head);

#endif
