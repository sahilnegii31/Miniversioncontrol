#include "iostream"
#include <fstream>
#include <sys/stat.h>
#include "login.h"
#include "commit.h"
#include "staging.h"
#include "revert.h"
#include "branch.h"
#include "shell.h"

using namespace std;

void ensureWorkingDir(string username) {
    mkdir("data", 0755);
    mkdir("data/working", 0755);
    string path = "data/working/" + username;
    mkdir(path.c_str(), 0755);
}

void listBranchesMenu(CommitNode* head, const string& currentBranch) {
    vector<BranchDetail> branches = getAllBranchesDetailed(head);

    cout << "\n================== Branches in MiniVCS ==================\n";
    for (const auto& b : branches) {
        bool isActive = (b.name == currentBranch);
        cout << (isActive ? " * " : "   ") << b.name;
        if (isActive) cout << " [ACTIVE]";
        cout << "\n   Folder : data/branches/" << b.name << "/ (" << b.fileCount << " file" << (b.fileCount == 1 ? "" : "s") << ")";
        cout << "\n   Files  : ";
        if (b.files.empty()) {
            cout << "(empty)";
        } else {
            for (size_t i = 0; i < b.files.size(); i++) {
                cout << b.files[i] << (i + 1 < b.files.size() ? ", " : "");
            }
        }
        cout << "\n   Latest : " << b.latestCommitId << " (" << b.latestCommitMsg << ")\n";
        cout << "---------------------------------------------------------\n";
    }

    getGlobalBranchTree().displayTree();
}

void createBranchMenu(string username, string& currentBranch) {
    cout << "Enter new branch name: ";
    string bName;
    cin >> bName;

    cout << "Branch off from [" << currentBranch << "]: ";
    string fromBranch;
    cin.ignore();
    getline(cin, fromBranch);
    if (fromBranch.empty()) fromBranch = currentBranch;

    bool ok = createBranch(bName, fromBranch, username);
    if (ok) {
        cout << "Branch '" << bName << "' created successfully!\n";
        cout << "Switch to '" << bName << "' now? (y/n): ";
        char ch;
        cin >> ch;
        if (ch == 'y' || ch == 'Y') {
            switchBranch(username, bName);
            currentBranch = bName;
            cout << "Switched to branch: " << currentBranch << "\n";
        }
    } else {
        cout << "Error: Could not create branch. Check if name is valid.\n";
    }
}

void switchBranchMenu(string username, string& currentBranch) {
    cout << "Current Branch: " << currentBranch << "\n";
    cout << "Enter target branch: ";
    string target;
    cin >> target;

    if (target == currentBranch) {
        cout << "Already on branch: " << currentBranch << "\n";
        return;
    }

    bool ok = switchBranch(username, target);
    if (ok) {
        currentBranch = target;
        cout << "Switched to branch: " << currentBranch << "\n";
        cout << "Working directory synchronized with data/branches/" << target << "/\n";
    } else {
        cout << "Failed to switch to branch '" << target << "'.\n";
    }
}

void writeFileMenu(string username, const string& currentBranch) {
    cout << "Enter file name: ";
    string fname;
    cin >> fname;

    cout << "Enter file content: ";
    cin.ignore();
    string content;
    getline(cin, content);

    string wPath = "data/working/" + username + "/" + fname;
    ofstream wf(wPath.c_str());
    if (wf.is_open()) {
        wf << content;
        wf.close();

        ensureBranchDir(currentBranch);
        string bPath = "data/branches/" + currentBranch + "/" + fname;
        ofstream bf(bPath.c_str());
        if (bf.is_open()) {
            bf << content;
            bf.close();
        }

        cout << "Saved: " << fname << " (synced to branch: " << currentBranch << ")\n";
    } else {
        cout << "Could not save file.\n";
    }
}

void addFileToStaging(StagingArea &staging, string username, const string& currentBranch) {
    syncWorkingWithBranch(username, currentBranch);

    cout << "Enter file name to stage: ";
    string fname;
    cin >> fname;

    string path = "data/working/" + username + "/" + fname;
    ifstream f(path.c_str());

    if (!f.is_open()) {
        cout << "File not found: " << path << "\n";
        return;
    }

    string content = "", line;
    while (getline(f, line)) {
        content += line + "\n";
    }
    f.close();

    staging.stageFile(fname, content);
    cout << "Staged: " << fname << " (ready for commit)\n";
}

void mainMenu(string username) {
    ensureWorkingDir(username);

    CommitNode* commitHead = loadCommits();
    initBranchSystem(commitHead);
    string currentBranch = getUserCurrentBranch(username);
    syncWorkingWithBranch(username, currentBranch);
    StagingArea staging;

    int choice;
    do {
        cout << "\n=======================================================\n";
        cout << "                     MINIVCS CLI                       \n";
        cout << "\nUser: @" << username << "          Active Branch: [" << currentBranch << "]\n";
        cout << "=======================================================\n";
        cout << " [ WORKING DIRECTORY & STAGING ]\n";
        cout << "  1. Write / Edit File       2. Stage File (git add)\n";
        cout << "  3. View Staging Area       4. Unstage Last File\n\n";
        cout << " \n[ COMMITS & HISTORY ]\n";
        cout << "  5. Commit Staged Files     6. View Commit History (git log)\n";
        cout << "  7. Inspect Commit Diff     8. Checkout Commit\n";
        cout << "  9. Undo Last Commit       10. Revert to Commit\n\n";
        cout << " \n[ BRANCH MANAGEMENT ]\n";
        cout << " 11. View Branches & Tree   12. Create New Branch\n";
        cout << " 13. Switch Branch (checkout)\n\n";
        cout << "\n[ INTERACTIVE SHELL ] \n\n";
        cout << " 14. Run Interactive Shell\n\n";
        cout << "  0. Logout & Exit\n";
        cout << "=======================================================\n";
        cout << "Choice > ";
        cin >> choice;

        switch (choice) {
            case 1:
                writeFileMenu(username, currentBranch);
                break;

            case 2:
                addFileToStaging(staging, username, currentBranch);
                break;

            case 3:
                staging.viewStaging();
                break;

            case 4:
                staging.unstageLastFile();
                break;

            case 5:
                commitHead = staging.commitAll(commitHead, username, currentBranch);
                break;

            case 6:
                showHistory(commitHead);
                break;

            case 7: {
                cout << "Enter commit ID to inspect: ";
                string commitId;
                cin >> commitId;
                viewCommitChanges(commitHead, commitId, username);
                break;
            }

            case 8: {
                cout << "Enter commit ID to checkout: ";
                string commitId;
                cin >> commitId;
                checkoutCommit(commitHead, commitId, username);
                break;
            }

            case 9:
                commitHead = undoLastCommit(commitHead, username);
                break;

            case 10: {
                cout << "Enter commit ID to revert to: ";
                string targetId;
                cin >> targetId;
                commitHead = revertToCommit(commitHead, targetId, username);
                break;
            }

            case 11:
                listBranchesMenu(commitHead, currentBranch);
                break;

            case 12:
                createBranchMenu(username, currentBranch);
                break;

            case 13:
                switchBranchMenu(username, currentBranch);
                break;
            case 14 :
                startShell();
            case 0:
                cout << "Logged out successfully. Goodbye, " << username << "!\n";
                break;

            default:
                cout << "Invalid choice. Please select from menu.\n";
        }

    } while (choice != 0);

    freeCommits(commitHead);
}

int main() {
    cout << "======================================\n";
    cout << "    MiniVCS Version Control System    \n";
    cout << "======================================\n";

    string username = runLoginMenu();
    mainMenu(username);

    return 0;
}
