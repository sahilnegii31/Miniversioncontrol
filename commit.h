#ifndef COMMIT_H
#define COMMIT_H

#include <string>
using namespace std;

struct CommitNode {
    string commitId;
    string author;
    string message;
    string timestamp;
    string branch;
    string parentId;

    CommitNode* next;

    CommitNode() : next(nullptr) {}
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
