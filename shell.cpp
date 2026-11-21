#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include "shell.h"
#include "staging.h"
#include "commit.h"
#include "branch.h"
#include "revert.h"
#include "login.h"

using namespace std;

// Create an empty file in user's working directory
void createFile(string username, string filename) {
    string wPath = "data/working/" + username + "/" + filename;
    if (filesystem::exists(wPath)) {
        cout << "File already exists: " << filename << endl;
        return;
    }
    ofstream wf(wPath.c_str());
    wf.close();
    cout << "File created: " << filename << endl;
}

// Write content to a file in user's working directory
void writeFile(string username, string filename) {
    string wPath = "data/working/" + username + "/" + filename;
    cout << "Enter content for " << filename << " (press Enter to save):\n> ";
    string content;
    getline(cin, content);

    ofstream wf(wPath.c_str());
    wf << content << "\n";
    wf.close();
    cout << "Saved: " << filename << endl;
}

// Display contents of a file
void readFile(string username, string filename) {
    string wPath = "data/working/" + username + "/" + filename;
    ifstream rf(wPath.c_str());
    if (!rf.is_open()) {
        cout << "File not found: " << filename << endl;
        return;
    }
    string line;
    cout << "--- " << filename << " ---\n";
    while (getline(rf, line)) {
        cout << line << endl;
    }
    cout << "---------------------\n";
    rf.close();
}

// List all files in the current branch
void listFiles(string currentBranch) {
    vector<string> files = getBranchFiles(currentBranch);
    cout << "Files on branch [" << currentBranch << "]:\n";
    if (files.empty()) {
        cout << "  (no files)\n";
    } else {
        for (string f : files) {
            cout << "  - " << f << endl;
        }
    }
}

// Interactive Custom Shell
void startShell(string username) {
    if (username.empty()) {
        username = runLoginMenu();
    }

    CommitNode* commitHead = loadCommits();
    initBranchSystem(commitHead);
    string currentBranch = getUserCurrentBranch(username);
    syncWorkingWithBranch(username, currentBranch);
    StagingArea staging;

    cout << "\n========================================\n";
    cout << "          MiniVCS Custom Shell\n";
    cout << "  Type 'help' to see all commands\n";
    cout << "  User: @" << username << " | Branch: [" << currentBranch << "]\n";
    cout << "========================================\n\n";

    string command;

    while (true) {
        cout << "minivcs (" << currentBranch << ")> ";
        getline(cin, command);

        if (command.empty()) {
            continue;
        }

        // Exit shell
        if (command == "exit" || command == "quit") {
            cout << "Exiting MiniVCS Shell...\n";
            break;
        }

        // Help menu
        else if (command == "help") {
            cout << "\n================ MiniVCS Shell Help ================\n";
            cout << "  create <file>         - Create a new file\n";
            cout << "  write <file>          - Write text into a file\n";
            cout << "  cat <file>            - View file contents\n";
            cout << "  ls                    - List files in current branch\n";
            cout << "  add <file>            - Stage a file for commit\n";
            cout << "  status                - View staged files\n";
            cout << "  commit -m <msg>       - Commit staged files with message\n";
            cout << "  log                   - View commit history\n";
            cout << "  undo                  - Undo the last commit\n";
            cout << "  branch                - List all branches\n";
            cout << "  branch <name>         - Create a new branch\n";
            cout << "  checkout <branch>     - Switch to another branch\n";
            cout << "  logout                - Switch user / log in\n";
            cout << "  exit                  - Exit shell\n";
            cout << "====================================================\n\n";
        }

        // Create file
        else if (command.find("create ") == 0) {
            string filename = command.substr(7);
            createFile(username, filename);
        }

        // Write content to file
        else if (command.find("write ") == 0) {
            string filename = command.substr(6);
            writeFile(username, filename);
        }

        // View file content
        else if (command.find("cat ") == 0) {
            string filename = command.substr(4);
            readFile(username, filename);
        }

        // List files
        else if (command == "ls" || command == "dir") {
            listFiles(currentBranch);
        }

        // Stage file (git add)
        else if (command.find("add ") == 0) {
            string filename = command.substr(4);
            string wPath = "data/working/" + username + "/" + filename;
            ifstream rf(wPath.c_str());
            if (!rf.is_open()) {
                cout << "File not found: " << filename << endl;
            } else {
                string content = "", line;
                while (getline(rf, line)) {
                    content += line + "\n";
                }
                rf.close();
                staging.stageFile(filename, content);
                cout << "Staged: " << filename << endl;
            }
        }

        // View staging status
        else if (command == "status") {
            staging.viewStaging();
        }

        // Commit staged files
        else if (command.find("commit -m ") == 0) {
            string msg = command.substr(10);
            if (msg.empty()) {
                cout << "Please provide a commit message!\n";
            } else if (staging.isEmpty()) {
                cout << "Staging area is empty. Stage a file first using 'add <file>'.\n";
            } else {
                commitHead = staging.commitAllWithMessage(commitHead, username, currentBranch, msg);
            }
        }

        // Show commit log
        else if (command == "log" || command == "history") {
            showHistory(commitHead);
        }

        // Undo last commit
        else if (command == "undo") {
            commitHead = undoLastCommit(commitHead, username);
        }

        // List branches
        else if (command == "branch") {
            vector<string> branches = listAllBranches();
            cout << "Branches in MiniVCS:\n";
            for (string b : branches) {
                if (b == currentBranch) cout << " * " << b << " [current]\n";
                else cout << "   " << b << "\n";
            }
        }

        // Create new branch
        else if (command.find("branch ") == 0) {
            string bName = command.substr(7);
            if (createBranch(bName, currentBranch, username)) {
                cout << "Branch '" << bName << "' created successfully!\n";
            } else {
                cout << "Failed to create branch: " << bName << endl;
            }
        }

        // Switch branch
        else if (command.find("checkout ") == 0) {
            string target = command.substr(9);
            if (switchBranch(username, target)) {
                currentBranch = target;
                cout << "Switched to branch: " << currentBranch << endl;
            } else {
                cout << "Failed to switch to branch: " << target << endl;
            }
        }

        // Switch user / logout
        else if (command == "logout" || command == "login") {
            username = runLoginMenu();
            currentBranch = getUserCurrentBranch(username);
            syncWorkingWithBranch(username, currentBranch);
            cout << "Now logged in as: @" << username << " on branch [" << currentBranch << "]\n";
        }

        // Unknown command
        else {
            cout << "Unknown command: '" << command << "'. Type 'help' to see all commands.\n";
        }
    }
}
