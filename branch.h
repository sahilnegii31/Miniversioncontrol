#ifndef BRANCH_H
#define BRANCH_H

#include <iostream>
#include <string>
#include <vector>
#include "commit.h"

using namespace std;

// BranchDetail class to hold branch info for display and API responses
class BranchDetail {
public:
    string name;
    string parentBranch;
    string latestCommitId;
    string latestCommitMsg;
    string lastUpdated;
    int fileCount;
    vector<string> files;

    BranchDetail() : fileCount(0) {}
};

// TreeNode class representing a node in the Branch Tree data structure
class TreeNode {
public:
    string name;
    string parentBranch;
    vector<TreeNode*> children;

    TreeNode(string bName, string pName = "") {
        name = bName;
        parentBranch = pName;
    }
};

// General Tree to represent branch hierarchy
class BranchTree {
private:
    TreeNode* root;

    TreeNode* search(TreeNode* curr, string bName);
    void print(TreeNode* curr, int depth);
    void destroy(TreeNode* curr);

public:
    BranchTree();
    ~BranchTree();

    TreeNode* find(string bName);
    bool exists(string bName);
    void insert(string bName, string parentName = "main");
    void displayTree();
};

BranchTree& getGlobalBranchTree();

void makeDir(string path);
bool dirExists(string path);
bool copyFile(string src, string dst);
void clearDirectory(string dirPath);
void copyDirectory(string srcDir, string dstDir);

vector<string> getBranchFiles(string branchName);
string readBranchFileContent(string branchName, string filename);
void ensureBranchDir(string branchName);
void syncWorkingWithBranch(string username, string branchName);

string getUserCurrentBranch(string username);
void setUserCurrentBranch(string username, string branchName);

vector<string> listAllBranches();
void syncBranchWithCommit(string branchName, string commitId);
void initBranchSystem(CommitNode* head);
vector<BranchDetail> getAllBranchesDetailed(CommitNode* head);

bool createBranch(string newBranch, string fromBranch = "main", string creator = "sahil");
bool switchBranch(string username, string targetBranch);
bool removeBranch(string branchName, string& errorMsg);

#endif // BRANCH_H
