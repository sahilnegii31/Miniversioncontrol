#include "branch.h"
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

using namespace std;

// --- BranchTree (Tree Data Structure) Implementation ---

BranchTree::BranchTree() {
    root = new TreeNode("main", "");
}

BranchTree::~BranchTree() {
    destroy(root);
}

// Search for a branch node recursively in the tree
TreeNode* BranchTree::search(TreeNode* curr, string bName) {
    if (curr == nullptr) return nullptr;
    if (curr->name == bName) return curr;

    for (TreeNode* child : curr->children) {
        TreeNode* found = search(child, bName);
        if (found != nullptr) return found;
    }
    return nullptr;
}

// Print branch hierarchy with indentation
void BranchTree::print(TreeNode* curr, int depth) {
    if (curr == nullptr) return;

    for (int i = 0; i < depth; i++) cout << "    ";
    if (depth > 0) cout << "|-- ";
    cout << curr->name;
    if (!curr->parentBranch.empty()) {
        cout << " (from " << curr->parentBranch << ")";
    }
    cout << "\n";

    for (TreeNode* child : curr->children) {
        print(child, depth + 1);
    }
}

// Free all tree nodes post-order
void BranchTree::destroy(TreeNode* curr) {
    if (curr == nullptr) return;
    for (TreeNode* child : curr->children) {
        destroy(child);
    }
    delete curr;
}

TreeNode* BranchTree::find(string bName) {
    return search(root, bName);
}

bool BranchTree::exists(string bName) {
    return find(bName) != nullptr;
}

void BranchTree::insert(string bName, string parentName) {
    if (exists(bName)) return;

    TreeNode* parentNode = find(parentName);
    if (parentNode == nullptr) parentNode = root;

    TreeNode* newNode = new TreeNode(bName, parentNode ? parentNode->name : "main");
    parentNode->children.push_back(newNode);
}

void BranchTree::displayTree() {
    cout << "\n================ Branch Tree ================\n";
    print(root, 0);
    cout << "=============================================\n";
}

BranchTree& getGlobalBranchTree() {
    static BranchTree tree;
    return tree;
}

// --- Directory and File Helper Functions ---

void makeDir(string path) {
    mkdir(path.c_str(), 0755);
}

bool dirExists(string path) {
    DIR* d = opendir(path.c_str());
    if (d != nullptr) {
        closedir(d);
        return true;
    }
    return false;
}

bool copyFile(string src, string dst) {
    ifstream in(src.c_str(), ios::binary);
    if (!in.is_open()) return false;

    ofstream out(dst.c_str(), ios::binary);
    if (!out.is_open()) return false;

    out << in.rdbuf();
    return true;
}

void clearDirectory(string dirPath) {
    DIR* dir = opendir(dirPath.c_str());
    if (dir == nullptr) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string fname = entry->d_name;
        if (fname != "." && fname != "..") {
            string fullPath = dirPath + "/" + fname;
            unlink(fullPath.c_str());
        }
    }
    closedir(dir);
}

void copyDirectory(string srcDir, string dstDir) {
    makeDir(dstDir);
    DIR* dir = opendir(srcDir.c_str());
    if (dir == nullptr) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string fname = entry->d_name;
        if (fname != "." && fname != "..") {
            copyFile(srcDir + "/" + fname, dstDir + "/" + fname);
        }
    }
    closedir(dir);
}

// --- Branch File Operations ---

vector<string> getBranchFiles(string branchName) {
    vector<string> fileList;
    string bDir = "data/branches/" + branchName;
    DIR* dir = opendir(bDir.c_str());
    if (dir == nullptr) return fileList;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string fname = entry->d_name;
        if (fname != "." && fname != "..") {
            fileList.push_back(fname);
        }
    }
    closedir(dir);
    sort(fileList.begin(), fileList.end());
    return fileList;
}

string readBranchFileContent(string branchName, string filename) {
    string filePath = "data/branches/" + branchName + "/" + filename;
    ifstream f(filePath.c_str());
    if (!f.is_open()) return "";

    ostringstream oss;
    oss << f.rdbuf();
    return oss.str();
}

void ensureBranchDir(string branchName) {
    makeDir("data");
    makeDir("data/branches");
    if (!branchName.empty()) {
        makeDir("data/branches/" + branchName);
    }
}

// Sync files between a user's working directory and active branch folder
void syncWorkingWithBranch(string username, string branchName) {
    if (username.empty() || branchName.empty()) return;

    string workDir   = "data/working/" + username;
    string branchDir = "data/branches/" + branchName;

    makeDir("data");
    makeDir("data/working");
    makeDir(workDir);
    ensureBranchDir(branchName);

    // 1. Copy files from branch to working directory (if missing in working)
    DIR* bDir = opendir(branchDir.c_str());
    if (bDir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(bDir)) != nullptr) {
            string fname = entry->d_name;
            if (fname != "." && fname != "..") {
                string wFile = workDir + "/" + fname;
                ifstream test(wFile.c_str());
                if (!test.is_open()) {
                    copyFile(branchDir + "/" + fname, wFile);
                }
            }
        }
        closedir(bDir);
    }

    // 2. Copy working files to branch folder
    DIR* wDir = opendir(workDir.c_str());
    if (wDir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(wDir)) != nullptr) {
            string fname = entry->d_name;
            if (fname != "." && fname != "..") {
                copyFile(workDir + "/" + fname, branchDir + "/" + fname);
            }
        }
        closedir(wDir);
    }
}

// --- Active Branch Tracking (Per User) ---

static map<string, string> activeUserBranches;

static void loadActiveBranches() {
    ifstream f("data/active_branches.txt");
    if (!f.is_open()) return;

    string line;
    while (getline(f, line)) {
        size_t colon = line.find(':');
        if (colon != string::npos) {
            string u = line.substr(0, colon);
            string b = line.substr(colon + 1);
            if (!u.empty() && !b.empty()) {
                activeUserBranches[u] = b;
            }
        }
    }
    f.close();
}

static void saveActiveBranches() {
    makeDir("data");
    ofstream f("data/active_branches.txt");
    if (!f.is_open()) return;

    for (const auto& pair : activeUserBranches) {
        f << pair.first << ":" << pair.second << "\n";
    }
    f.close();
}

string getUserCurrentBranch(string username) {
    if (activeUserBranches.empty()) {
        loadActiveBranches();
    }
    if (activeUserBranches.find(username) != activeUserBranches.end()) {
        return activeUserBranches[username];
    }
    return "main";
}

void setUserCurrentBranch(string username, string branchName) {
    activeUserBranches[username] = branchName;
    saveActiveBranches();
}

// --- Branch Management Operations ---

// List all branch folder names under data/branches
vector<string> listAllBranches() {
    vector<string> result;
    result.push_back("main");

    DIR* dir = opendir("data/branches");
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            string name = entry->d_name;
            if (name != "." && name != ".." && name != "main") {
                string fullPath = "data/branches/" + name;
                if (dirExists(fullPath)) {
                    result.push_back(name);
                }
            }
        }
        closedir(dir);
    }
    return result;
}

// Update branch folder with the contents of a commit snapshot
void syncBranchWithCommit(string branchName, string commitId) {
    if (branchName.empty() || commitId.empty()) return;

    string branchDir = "data/branches/" + branchName;
    string objDir = "data/objects/" + commitId;
    if (!dirExists(objDir)) return;

    ensureBranchDir(branchName);
    clearDirectory(branchDir);
    copyDirectory(objDir, branchDir);
}

// Initialize branches on application launch
void initBranchSystem(CommitNode* head) {
    makeDir("data");
    makeDir("data/branches");
    ensureBranchDir("main");

    loadActiveBranches();

    // Reconstruct branch tree from commits
    CommitNode* curr = head;
    while (curr != nullptr) {
        if (!curr->branch.empty()) {
            ensureBranchDir(curr->branch);
            getGlobalBranchTree().insert(curr->branch, "main");
        }
        curr = curr->next;
    }

    // If main is empty but sahil has working files, seed main
    vector<string> mainFiles = getBranchFiles("main");
    if (mainFiles.empty() && dirExists("data/working/sahil")) {
        copyDirectory("data/working/sahil", "data/branches/main");
    }
}

// Return branch details including latest commit and file list
vector<BranchDetail> getAllBranchesDetailed(CommitNode* head) {
    vector<string> bNames = listAllBranches();
    vector<BranchDetail> details;

    for (const string& b : bNames) {
        BranchDetail bd;
        bd.name = b;
        bd.parentBranch = "main";
        bd.latestCommitId = "NONE";
        bd.latestCommitMsg = "No commits yet on this branch";
        bd.lastUpdated = "Recently";

        TreeNode* node = getGlobalBranchTree().find(b);
        if (node && !node->parentBranch.empty()) {
            bd.parentBranch = node->parentBranch;
        }

        // Find latest commit on this branch from linked list
        CommitNode* curr = head;
        while (curr != nullptr) {
            if (curr->branch == b) {
                bd.latestCommitId  = curr->commitId;
                bd.latestCommitMsg = curr->message;
                bd.lastUpdated     = curr->timestamp;
            }
            curr = curr->next;
        }

        bd.files = getBranchFiles(b);
        bd.fileCount = (int)bd.files.size();
        details.push_back(bd);
    }

    return details;
}

// Create a new branch by copying files from parent branch
bool createBranch(string newBranch, string fromBranch, string creator) {
    if (newBranch.empty()) return false;

    // Validate characters: alphanumeric, hyphen, underscore
    for (char c : newBranch) {
        if (!isalnum(c) && c != '-' && c != '_') return false;
    }

    ensureBranchDir(newBranch);
    string targetDir = "data/branches/" + newBranch;
    string sourceDir = "data/branches/" + fromBranch;

    if (!dirExists(sourceDir)) {
        sourceDir = "data/branches/main";
    }

    if (dirExists(sourceDir)) {
        copyDirectory(sourceDir, targetDir);
    } else {
        string workDir = "data/working/" + creator;
        if (dirExists(workDir)) {
            copyDirectory(workDir, targetDir);
        }
    }

    // Insert into Branch Tree
    getGlobalBranchTree().insert(newBranch, fromBranch);
    return true;
}

// Switch user active branch and sync working directory
bool switchBranch(string username, string targetBranch) {
    if (username.empty() || targetBranch.empty()) return false;

    ensureBranchDir(targetBranch);
    string branchDir = "data/branches/" + targetBranch;
    string workDir   = "data/working/" + username;

    makeDir("data/working");
    makeDir(workDir);

    clearDirectory(workDir);
    copyDirectory(branchDir, workDir);

    setUserCurrentBranch(username, targetBranch);
    return true;
}

// Delete a branch folder
bool removeBranch(string branchName, string& errorMsg) {
    if (branchName == "main") {
        errorMsg = "Cannot delete the default 'main' branch.";
        return false;
    }

    string bDir = "data/branches/" + branchName;
    if (!dirExists(bDir)) {
        errorMsg = "Branch does not exist: " + branchName;
        return false;
    }

    clearDirectory(bDir);
    rmdir(bDir.c_str());

    // Reset user if they were on this branch
    for (auto& pair : activeUserBranches) {
        if (pair.second == branchName) {
            pair.second = "main";
        }
    }
    saveActiveBranches();

    return true;
}
