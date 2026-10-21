#ifndef BRANCH_H
#define BRANCH_H

#include <iostream>
#include <string>
#include <vector>
#include "commit.h"

using namespace std;

// Struct to hold branch info for display and API responses
struct BranchDetail {
    string name;
    string parentBranch;
    string latestCommitId;
    string latestCommitMsg;
    string lastUpdated;
    int fileCount;
    vector<string> files;
};

// Node in the Branch Tree data structure
struct TreeNode {
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

// Access the single global branch tree
BranchTree& getGlobalBranchTree();

// File & directory utility functions
void makeDir(string path);
bool dirExists(string path);
bool copyFile(string src, string dst);
void clearDirectory(string dirPath);
void copyDirectory(string srcDir, string dstDir);

// Branch file operations
vector<string> getBranchFiles(string branchName);
string readBranchFileContent(string branchName, string filename);
void ensureBranchDir(string branchName);
void syncWorkingWithBranch(string username, string branchName);

// User active branch management
string getUserCurrentBranch(string username);
void setUserCurrentBranch(string username, string branchName);

// Branch lifecycle operations
vector<string> listAllBranches();
void syncBranchWithCommit(string branchName, string commitId);
void initBranchSystem(CommitNode* head);
vector<BranchDetail> getAllBranchesDetailed(CommitNode* head);

bool createBranch(string newBranch, string fromBranch = "main", string creator = "sahil");
bool switchBranch(string username, string targetBranch);
bool removeBranch(string branchName, string& errorMsg);

#endif // BRANCH_H
