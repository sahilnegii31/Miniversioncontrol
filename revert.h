#ifndef REVERT_H
#define REVERT_H

#include "commit.h"
#include <string>
using namespace std;

CommitNode* undoLastCommit(CommitNode* head, string currentUser);
CommitNode* revertToCommit(CommitNode* head, string targetId, string currentUser);
bool restoreSnapshot(string commitId, string username);
bool checkoutCommit(CommitNode* head, string commitId, string currentUser);

CommitNode* undoLastCommitDirect(CommitNode* head, string currentUser);
CommitNode* revertToCommitDirect(CommitNode* head, string targetId, string currentUser);
bool checkoutCommitDirect(CommitNode* head, string commitId, string currentUser);

#endif
