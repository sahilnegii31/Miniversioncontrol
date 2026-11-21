# MiniVCS 🚀
### A Lightweight, Educational Version Control System in C++ & Modern React
> **Designed for 3rd-Semester Computer Science Students | Clean, Human-Readable & Viva-Ready**

---

## 📖 Table of Contents
1. [Project Overview & Philosophy](#1-project-overview--philosophy)
2. [Core Data Structures Explained](#2-core-data-structures-explained)
3. [Physical Folder Architecture](#3-physical-folder-architecture)
4. [Compilation & How to Run](#4-compilation--how-to-run)
5. [Deep Dive: Branch Module (`branch.h` & `branch.cpp`)](#5-deep-dive-branch-module-branchh--branchcpp)
   * [5.1 The N-ary Tree Model](#51-the-n-ary-tree-model)
   * [5.2 Simple Tree Traversals (Search, Print, Destroy)](#52-simple-tree-traversals-search-print-destroy)
   * [5.3 Physical Directory Snapshot Isolation](#53-physical-directory-snapshot-isolation)
   * [5.4 Synchronization Engine (`syncWorkingWithBranch`)](#54-synchronization-engine-syncworkingwithbranch)
6. [Detailed Code Walkthrough by Module](#6-detailed-code-walkthrough-by-module)
   * [6.1 `stack.h` — Generic LIFO Stack](#61-stackh--generic-lifo-stack)
   * [6.2 `commit.h` & `commit.cpp` — Singly Linked List for Commits](#62-commith--commitcpp--singly-linked-list-for-commits)
   * [6.3 `staging.h` & `staging.cpp` — Staging Area Management](#63-stagingh--stagingcpp--staging-area-management)
   * [6.4 `branch.h` & `branch.cpp` — Complete Function Reference](#64-branchh--branchcpp--complete-function-reference)
   * [6.5 `revert.h` & `revert.cpp` — Time-Travel, Undo & Revert](#65-reverth--revertcpp--time-travel-undo--revert)
   * [6.6 `shell.h` & `shell.cpp` — Interactive Custom Shell](#66-shellh--shellcpp--interactive-custom-shell)
   * [6.7 `login.h` & `login.cpp` — User Authentication](#67-loginh--logincpp--user-authentication)
   * [6.8 `contribution.h` & `contribution.cpp` — Analytics Engine](#68-contributionh--contributioncpp--analytics-engine)
   * [6.9 `c_chart.h` & `c_chart.cpp` — Native C++ SVG Chart Generator](#69-c_charth--c_chartcpp--native-c-svg-chart-generator)
   * [6.10 `main.cpp` — Terminal CLI & Menu System](#610-maincpp--terminal-cli--menu-system)
   * [6.11 `server.cpp` — POSIX HTTP REST API Server](#611-servercpp--posix-http-rest-api-server)
   * [6.12 Frontend Architecture (React + Vite)](#612-frontend-architecture-react--vite)
7. [Top Viva Questions & Answers](#7-top-viva-questions--answers)

---

## 1. Project Overview & Philosophy

**MiniVCS** is an educational version control system inspired by Git, written from scratch in standard C++17 with an embedded HTTP REST server and a modern React web interface.

Unlike Git (which uses complex cryptographic SHA packfiles, index binary trees, and compressed blob storage), **MiniVCS is written in clean, humanized, understandable code** suitable for a 3rd-semester undergraduate student. It avoids convoluted template metaprogramming, external heavy libraries, or confusing pointer arithmetic, and instead uses **standard, textbook Data Structures & Algorithms (DSA)** to solve real version control challenges.

### The 4 Stages of Version Control in MiniVCS:
1. **Working Directory (`data/working/<user>/`)**: Your personal workspace where you create and edit code files.
2. **Staging Area (`Stack<StagedFile>`)**: A LIFO stack where files are gathered (`git add`) before being recorded permanently.
3. **Commit History (`CommitNode*` Linked List)**: A permanent, timestamped chain of snapshots (`git commit`).
4. **Physical Branch Tree (`BranchTree`, `data/branches/<branch>/`)**: An N-ary hierarchical tree where each branch has its own physical folder on disk.

---

## 2. Core Data Structures Explained

| Component | Data Structure | Why This Data Structure? | Time Complexity |
| :--- | :--- | :--- | :--- |
| **Staging Area** | **LIFO Stack** (`Stack<T>`) | Staging files follows a Last-In-First-Out model. Unstaging (`git restore --staged`) naturally pops the top element first. | Push: $O(1)$<br>Pop: $O(1)$ |
| **Commit History** | **Singly Linked List** (`CommitNode*`) | Commits happen sequentially over time. Each commit points to its parent (`parentId`), creating a chain that is easy to traverse and revert. | Insertion: $O(1)$<br>Traversal: $O(N)$ |
| **Branch Hierarchy** | **Manual N-ary Tree** (`TreeNode*`, `BranchTree`) | Branches branch off parent branches (`main` $\to$ `feature` $\to$ `dev`). A general tree where a node can have $N$ children is the exact mathematical model. | Search: $O(B)$<br>Insert: $O(1)$ |
| **User Directory** | **Array of Objects** (`User users[]`) | Bounded in-memory storage of usernames and passwords read and written to `data/users.txt`. | Lookup: $O(U)$<br>Save: $O(U)$ |
| **Analytics & Heatmap** | **Lookup Map** (`std::map<string, int>`) | Groups commit counts by date (`YYYY-MM-DD`) and author in sorted order. | Insert: $O(\log D)$<br>Lookup: $O(\log D)$ |

---

## 3. Physical Folder Architecture

MiniVCS stores all data transparently inside the `data/` folder on your hard drive:

```text
minivcs2/
├── data/
│   ├── working/                      <-- Working directories for each user
│   │   ├── sahil/
│   │   │   └── notes.txt
│   │   └── mohit/
│   ├── objects/                      <-- Permanent commit snapshots
│   │   ├── C001/                     <-- All files as they existed in Commit C001
│   │   │   └── notes.txt
│   │   └── C002/
│   ├── branches/                     <-- PHYSICAL BRANCH DIRECTORIES
│   │   ├── main/                     <-- Files currently on 'main' branch
│   │   │   └── notes.txt
│   │   └── feature/                  <-- Isolated files on 'feature' branch
│   │       └── code.c
│   ├── commits.txt                   <-- Commit metadata file (ID|Author|Msg|Time|Branch|Parent)
│   ├── users.txt                     <-- User credentials (username:password)
│   └── active_branches.txt           <-- Current active branch for each user (user:branch)
```

---

## 4. Compilation & How to Run

### Option 1: Using `make` (Recommended)
```bash
# Compile both the CLI application and the Backend Server
make clean && make all

# Run the Terminal CLI
./minivcs

# Run the C++ REST API Server on port 8080
./server 8080
```

### Option 2: Direct Single Compiler Commands
```bash
# Build CLI:
clang++ -std=c++17 -Wall -O2 main.cpp commit.cpp staging.cpp revert.cpp login.cpp branch.cpp shell.cpp -o minivcs

# Build Server:
clang++ -std=c++17 -Wall -O2 commit.cpp staging.cpp revert.cpp login.cpp branch.cpp shell.cpp contribution.cpp c_chart.cpp server.cpp -o server
```

### Option 3: Running the React Web GUI
```bash
cd frontend
npm install
npm run dev      # Starts Vite dev server on http://localhost:3000 (proxies API to 8080)
```
*(Or access the pre-built web interface directly through `http://localhost:8080/` when `./server 8080` is running).*

---

## 5. Deep Dive: Branch Module (`branch.h` & `branch.cpp`)

The Branch Module is one of the proudest highlights of MiniVCS. It combines **textbook tree data structures** with **transparent OS filesystem operations**.

### 5.1 The N-ary Tree Model

In Git, branches are represented as branches splitting from other branches. In Computer Science, a binary tree only allows 2 children (left and right), but a git branch can have any number of child branches ($0, 1, 2, 3, \dots, N$).

Therefore, MiniVCS uses an **N-ary General Tree**:
```text
                 [ main ] (Root)
                 /      \
      [ feature-login ]  [ dev ]
            /               \
     [ auth-oauth ]     [ bugfix-ui ]
```

Each node in this tree is represented by the `TreeNode` class:
```cpp
class TreeNode {
public:
    string name;                    // Name of branch (e.g. "feature-login")
    string parentBranch;            // Branch it split from (e.g. "main")
    vector<TreeNode*> children;     // Pointers to child branches

    TreeNode(string bName, string pName = "") {
        name = bName;
        parentBranch = pName;
    }
};
```

---

### 5.2 Simple Tree Traversals (Search, Print, Destroy)

All tree operations in MiniVCS use clean, understandable recursive traversals:

#### 1. Search Traversal (`BranchTree::search`)
* **Algorithm**: Depth-First Search (DFS).
* **Working**: Checks if the current node matches `bName`. If not, it loops through the `children` vector and recursively searches each child subtree.
```cpp
TreeNode* BranchTree::search(TreeNode* curr, string bName) {
    if (curr == nullptr) return nullptr;
    if (curr->name == bName) return curr;

    for (TreeNode* child : curr->children) {
        TreeNode* found = search(child, bName);
        if (found != nullptr) return found;
    }
    return nullptr;
}
```

#### 2. Display Traversal (`BranchTree::print`)
* **Algorithm**: Pre-order Traversal with Indentation.
* **Working**: Prints the current node with indentation equal to `depth * 4` spaces. Then recursively calls `print(child, depth + 1)` on each child.
* **Output**:
  ```text
  main
      |-- feature-login (from main)
          |-- auth-oauth (from feature-login)
      |-- dev (from main)
  ```

#### 3. Cleanup Traversal (`BranchTree::destroy`)
* **Algorithm**: Post-order Traversal (Bottom-up deletion).
* **Working**: First destroys all child nodes recursively, and only then deletes `curr`. This guarantees zero dangling pointers and zero memory leaks.

---

### 5.3 Physical Directory Snapshot Isolation

While real Git stores objects inside packfiles, MiniVCS uses **Physical Directory Snapshot Isolation**:
* Every branch exists as a physical directory on disk at `data/branches/<branchName>/`.
* When a user creates branch `feature` from `main`:
  1. MiniVCS creates folder `data/branches/feature/`.
  2. MiniVCS copies all files from `data/branches/main/` into `data/branches/feature/`.
  3. MiniVCS adds `feature` as a child node of `main` in the `BranchTree`.
* When a user switches branch to `feature`:
  1. MiniVCS clears the user's working folder `data/working/<user>/`.
  2. MiniVCS copies all files from `data/branches/feature/` into `data/working/<user>/`.
  3. MiniVCS saves the active branch in `data/active_branches.txt`.

---

### 5.4 Synchronization Engine (`syncWorkingWithBranch`)

To allow developers to edit files either directly inside VS Code or through the web GUI, `syncWorkingWithBranch(username, branchName)` keeps the physical working folder and the active branch folder in perfect sync:
1. **Branch $\to$ Working**: If a file exists in `data/branches/<branch>/` but does not exist in `data/working/<user>/`, it is copied to the working folder.
2. **Working $\to$ Branch**: All files modified in `data/working/<user>/` are copied into `data/branches/<branch>/`.
This bi-directional synchronization ensures that whatever files you see on disk always match what the version control system tracks.

---

## 6. Detailed Code Walkthrough by Module

---

### 6.1 `stack.h` — Generic LIFO Stack
* **File Type**: Data Structure Header.
* **DSA Used**: Linked List-based Generic Stack (`StackNode<T>`, `Stack<T>`).

#### Structs & Classes:
1. `template <typename T> struct StackNode`
   * `T data`: Value stored in the node.
   * `StackNode* next`: Pointer to the node underneath.
2. `template <typename T> class Stack`
   * `StackNode<T>* top`: Pointer to the top of the stack.
   * `int count`: Number of elements currently in the stack.

#### Function Declarations & Working:
* `void push(T val)`
  * **Working**: Allocates a new `StackNode<T>`, points its `next` to `top`, and updates `top = newNode`. Increases `count`. Runs in $O(1)$.
* `T pop()`
  * **Working**: Checks if `isEmpty()`. If not, retrieves `top->data`, moves `top = top->next`, frees the old top node, decrements `count`, and returns the value. Runs in $O(1)$.
* `T peek()`
  * **Working**: Returns the value at `top->data` without removing it from the stack. Runs in $O(1)$.
* `bool isEmpty()`
  * **Working**: Returns `top == nullptr`. Runs in $O(1)$.
* `int size()`
  * **Working**: Returns `count`. Runs in $O(1)$.

---

### 6.2 `commit.h` & `commit.cpp` — Singly Linked List for Commits
* **File Type**: Commit History Module.
* **DSA Used**: Singly Linked List (`CommitNode`).

#### Classes Defined:
1. `class CommitNode`
   * `string commitId`: Unique identifier (e.g. `C001`, `C002`).
   * `string author`: Username of the committer.
   * `string message`: Commit message description.
   * `string timestamp`: Formatted date/time (`YYYY-MM-DD HH:MM:SS`).
   * `string branch`: Branch on which the commit was made.
   * `string parentId`: Commit ID of the parent commit (`NONE` for root).
   * `CommitNode* next`: Pointer to the subsequent commit in history.
2. `class CommitHistory`
   * Encapsulates the linked list with member functions `add()`, `load()`, `save()`, `show()`, `find()`, and `freeHistory()`.

#### Function Declarations & Working:
* `string getCurrentTime()`
  * **Declaration**: `string getCurrentTime();`
  * **Working**: Uses C standard library `time()` and `strftime()` to format system wall-clock time as `YYYY-MM-DD HH:MM:SS`.
* `string generateCommitId(CommitNode* head)`
  * **Declaration**: `string generateCommitId(CommitNode* head);`
  * **Working**: Traverses the commit linked list, extracts the maximum numeric suffix from IDs (`C001` $\to 1$), increments it, and formats it as `C%03d` (e.g. `C002`).
* `CommitNode* addCommit(CommitNode* head, string author, string message, string branch)`
  * **Declaration**: `CommitNode* addCommit(CommitNode* head, string author, string message, string branch);`
  * **Working**: Creates a new `CommitNode`, sets timestamp and sequential ID. If `head == nullptr`, sets `parentId = "NONE"` and returns the new node as head. Otherwise, traverses to the tail, sets `parentId = tail->commitId`, links `tail->next = newNode`, and returns `head`.
* `CommitNode* loadCommits()`
  * **Declaration**: `CommitNode* loadCommits();`
  * **Working**: Reads `data/commits.txt` line by line. Splits each line by the `|` delimiter into 6 fields, reconstructs `CommitNode` instances, and links them into a linked list in memory.
* `void saveCommits(CommitNode* head)`
  * **Declaration**: `void saveCommits(CommitNode* head);`
  * **Working**: Iterates through the linked list and writes each node as `commitId|author|message|timestamp|branch|parentId\n` to `data/commits.txt`.
* `void showHistory(CommitNode* head)`
  * **Declaration**: `void showHistory(CommitNode* head);`
  * **Working**: Implements `git log`. Iterates from `head` to `tail`, printing ID, author, message, time, branch, and parent.
* `CommitNode* findCommit(CommitNode* head, string commitId)`
  * **Declaration**: `CommitNode* findCommit(CommitNode* head, string commitId);`
  * **Working**: Linear search over the linked list matching `curr->commitId == commitId`. Returns pointer to node or `nullptr`.
* `void viewCommitChanges(CommitNode* head, string commitId, string currentUser)`
  * **Declaration**: `void viewCommitChanges(CommitNode* head, string commitId, string currentUser);`
  * **Working**: Compares the file snapshots in `data/objects/<commitId>/` against its parent `data/objects/<parentId>/`. Identifies whether files were `[ADDED]`, `[MODIFIED]`, `[UNCHANGED]`, or `[DELETED]`.
* `void freeCommits(CommitNode* head)`
  * **Declaration**: `void freeCommits(CommitNode* head);`
  * **Working**: Deallocates all nodes in the linked list using a temporary pointer traversal to avoid memory leaks.

---

### 6.3 `staging.h` & `staging.cpp` — Staging Area Management
* **File Type**: Staging Area Module.
* **DSA Used**: LIFO Stack of `StagedFile` objects.

#### Classes Defined:
1. `class StagedFile`
   * `string filename`: Name of staged file.
   * `string content`: File buffer content.
2. `class StagingArea`
   * Encapsulates `Stack<StagedFile> stagingStack`.

#### Function Declarations & Working:
* `void stageFile(string filename, string content)`
  * **Working**: If the file is already in the stack, updates its content in place. Otherwise, pushes a new `StagedFile(filename, content)` onto `stagingStack`.
* `void unstageLastFile()`
  * **Working**: Pops the most recently staged file from the top of the stack and reports it.
* `void viewStaging()`
  * **Working**: Temporarily pops and displays each file in `stagingStack` with its byte size, then restores the stack.
* `CommitNode* commitAllWithMessage(CommitNode* head, string author, string branch, const string& commitMsg)`
  * **Working**:
    1. Synchronizes active branch with working directory.
    2. Calls `addCommit` to add a new commit node.
    3. Creates directory `data/objects/<newId>/`.
    4. Copies parent branch directory contents into the new snapshot.
    5. Pops each file from `stagingStack` and writes its updated contents into `data/objects/<newId>/`.
    6. Calls `saveCommits(head)` to persist `data/commits.txt`.
    7. Copies the snapshot into `data/branches/<branch>/` and `data/working/<author>/`.
* `void clear()`
  * **Working**: Pops and empties all items from the stack.
* `bool isEmpty()` & `int getCount()`
  * **Working**: Returns true if stack is empty; returns element count.

---

### 6.4 `branch.h` & `branch.cpp` — Complete Function Reference

#### Classes Defined:
1. `class BranchDetail`: DTO holding branch name, parent branch, latest commit ID, latest commit message, timestamp, file count, and files list.
2. `class TreeNode`: N-ary Tree Node containing `name`, `parentBranch`, and `vector<TreeNode*> children`.
3. `class BranchTree`: General Tree manager containing root pointer `TreeNode* root`.

#### Functions in `branch.cpp`:
* `void makeDir(string path)`: Calls POSIX `mkdir(path.c_str(), 0755)` to safely create directories.
* `bool dirExists(string path)`: Checks if a directory exists using `opendir()`.
* `bool copyFile(string src, string dst)`: Copies file content byte-for-byte using binary streams `std::ifstream` and `std::ofstream`.
* `void clearDirectory(string dirPath)`: Iterates over files in `dirPath` using `readdir()` and unlinks each file without deleting the directory itself.
* `void copyDirectory(string srcDir, string dstDir)`: Reads files in `srcDir` and copies each file into `dstDir`.
* `vector<string> getBranchFiles(string branchName)`: Reads filenames inside `data/branches/<branchName>/` and returns a sorted vector of strings.
* `string readBranchFileContent(string branchName, string filename)`: Reads file content from `data/branches/<branchName>/<filename>` into a string.
* `void ensureBranchDir(string branchName)`: Guarantees that `data/`, `data/branches/`, and `data/branches/<branchName>/` exist on disk.
* `void syncWorkingWithBranch(string username, string branchName)`: Bi-directionally synchronizes files between `data/working/<username>/` and `data/branches/<branchName>/`.
* `string getUserCurrentBranch(string username)`: Returns the active branch for a user from `data/active_branches.txt` (defaults to `"main"`).
* `void setUserCurrentBranch(string username, string branchName)`: Updates the user's active branch in memory and saves to `data/active_branches.txt`.
* `vector<string> listAllBranches()`: Scans directory `data/branches/` using `readdir()` and returns a list of all existing branch names with `"main"` first.
* `void syncBranchWithCommit(string branchName, string commitId)`: Replaces the contents of `data/branches/<branchName>/` with the commit snapshot `data/objects/<commitId>/`.
* `void initBranchSystem(CommitNode* head)`: Reconstructs the in-memory `BranchTree` from commits, ensures `data/branches/main/` exists, and seeds initial files if needed.
* `vector<BranchDetail> getAllBranchesDetailed(CommitNode* head)`: Assembles full branch metadata (files, commit ID, commit message) for the CLI and Web REST API.
* `bool createBranch(string newBranch, string fromBranch, string creator)`:
  * Validates branch name (alphanumeric, hyphens, underscores).
  * Creates folder `data/branches/<newBranch>/`.
  * Copies files from parent branch `data/branches/<fromBranch>/`.
  * Inserts node into `BranchTree` as a child of `fromBranch`.
* `bool switchBranch(string username, string targetBranch)`:
  * Clears `data/working/<username>/`.
  * Clones files from `data/branches/<targetBranch>/` into the working directory.
  * Updates active branch tracking.
* `bool removeBranch(string branchName, string& errorMsg)`:
  * Deletes `data/branches/<branchName>/` and removes the branch from tracking. Safeguarded against deleting `"main"`.

---

### 6.5 `revert.h` & `revert.cpp` — Time-Travel, Undo & Revert
* **File Type**: Time Travel Engine.
* **DSA Used**: Singly Linked List node deletion and truncation.

#### Functions:
* `bool restoreSnapshot(string commitId, string username)`
  * **Working**: Copies all files from snapshot `data/objects/<commitId>/` into `data/working/<username>/`.
* `CommitNode* undoLastCommitDirect(CommitNode* head, string currentUser)`
  * **Working**: Non-interactive undo. Traverses to the second-to-last node, deletes the tail node, sets `prev->next = nullptr`, restores files from `parentId`, and updates `data/commits.txt`.
* `CommitNode* revertToCommitDirect(CommitNode* head, string targetId, string currentUser)`
  * **Working**: Non-interactive revert. Finds `targetId` in the list, deallocates all subsequent nodes, sets `target->next = nullptr`, restores files from `targetId`, and updates `data/commits.txt`.
* `bool checkoutCommit(CommitNode* head, string commitId, string currentUser)`
  * **Working**: Interactive CLI checkout prompting user confirmation before restoring files.
* `CommitNode* undoLastCommit(...)` & `CommitNode* revertToCommit(...)`
  * **Working**: Interactive CLI wrappers with `(y/n)` confirmations.

---

### 6.6 `shell.h` & `shell.cpp` — Interactive Custom Shell
* **File Type**: Interactive Terminal Shell.
* **Prompt**: `minivcs (<branch>)> `

#### Functions:
* `void createFile(string username, string filename)`: Creates a new empty file in `data/working/<username>/`.
* `void writeFile(string username, string filename)`: Prompts user to input text and writes it into the file.
* `void readFile(string username, string filename)`: Reads and displays the file contents on the terminal (`cat`).
* `void listFiles(string currentBranch)`: Displays files on the current branch (`ls`).
* `void startShell(string username)`: Runs the command interpreter loop supporting `help`, `create`, `write`, `cat`, `ls`, `add`, `status`, `commit -m`, `log`, `undo`, `branch`, `checkout`, `logout`, and `exit`.

---

### 6.7 `login.h` & `login.cpp` — User Authentication
* **File Type**: User Authentication Module.
* **DSA Used**: Array of `User` objects (`User users[MAX_USERS]`).

#### Functions:
* `int loadUsers(User users[], int maxUsers)`: Reads `username:password` lines from `data/users.txt`.
* `void saveUsers(User users[], int count)`: Writes `users[]` array back to `data/users.txt`.
* `bool registerUser(User users[], int &count)`: Prompts for new username and password, validates uniqueness, appends to array, and calls `saveUsers()`.
* `string loginUser(User users[], int count)`: Matches username and password. Returns logged-in username or empty string.
* `string runLoginMenu()`: Terminal menu loop with choices: `1. Login`, `2. Register`, `3. Exit`.

---

### 6.8 `contribution.h` & `contribution.cpp` — Analytics Engine
* **File Type**: GitHub-style Insights Analytics.
* **DSA Used**: `std::map<string, int>` (Red-Black Tree) for sorted date counts.

#### Functions:
* `ContributionSummary calculateContributions(CommitNode* head, const string& authorFilter)`:
  * Computes total commits, active days, current streak, longest streak, and author share percentages over a rolling 14-day window.
* `string generateContributionSvg(const ContributionSummary& summary)`:
  * Renders a vector SVG Commit Activity Histogram directly from C++.
* `string generateContributorSvg(const ContributionSummary& summary)`:
  * Renders a vector SVG Contributor Breakdown chart directly from C++.

---

### 6.9 `c_chart.h` & `c_chart.cpp` — Native C++ SVG Chart Generator
* **File Type**: Embedded Vector Graphics Generator.
* **Outputs**: Pure XML/SVG markup for histogram bars, axis labels, badges, and percentage progress bars without any external dependencies.

---

### 6.10 `main.cpp` — Terminal CLI & Menu System
* **File Type**: Command Line Application Entry Point.
* Provides a menu with 14 options covering working directory files, staging, commits, time-travel checkout, branch management, and the interactive shell (`Option 14`).

---

### 6.11 `server.cpp` — POSIX HTTP REST API Server
* **File Type**: Backend Web Server.
* **Socket Technology**: Standard Berkeley Sockets (`socket()`, `bind()`, `listen()`, `accept()`).
* **Port**: `8080`.
* **Endpoints**:
  * `GET /api/status`: Health check.
  * `POST /api/login`: Authenticate user credentials.
  * `POST /api/register`: Register new account.
  * `GET /api/files`: List files in working directory.
  * `GET /api/file`: Get file contents.
  * `POST /api/file`: Save file contents.
  * `DELETE /api/file`: Delete a file.
  * `GET /api/staging`: View staged files.
  * `POST /api/staging/add`: Stage a file (`git add`).
  * `POST /api/staging/unstage`: Unstage last file.
  * `POST /api/staging/clear`: Clear staging area.
  * `GET /api/commits`: List commit history.
  * `POST /api/commit`: Create new commit.
  * `POST /api/checkout`: Move working directory to past commit.
  * `POST /api/undo`: Undo last commit.
  * `POST /api/revert`: Truncate commit history back to target commit.
  * `GET /api/branches`: List all branches with files and commit info.
  * `POST /api/branch`: Create a new branch.
  * `POST /api/branch/switch`: Checkout/switch active branch.
  * `DELETE /api/branch`: Delete a branch.
  * `GET /api/branch/files`: List snapshot files in a branch directory.
  * `GET /api/branch/file`: Read content of a file in a branch directory.
  * `GET /api/contributions`: Retrieve insights metrics and embedded C++ SVG charts.

---

### 6.12 Frontend Architecture (React + Vite)
Located in `frontend/`, built with modern React components:
* `Navbar.jsx`: Brand banner, current branch indicator, active user profile pill, and tab switcher.
* `BranchesPage.jsx`:
  * Visual N-ary Tree hierarchy box.
  * Search/filter bar for branches.
  * Default `main` branch card with metadata and folder path.
  * Feature branches list with active status and parent branch lineage.
  * New Branch modal dialog.
  * Branch Files Inspector modal with split file-tree and preview panel.
* `WorkingDirExplorer.jsx`: Interactive file editor (create, edit, save, delete files).
* `StagingPanel.jsx`: Staged files stack viewer, commit message input, and commit trigger.
* `CommitHistory.jsx`: Commit history chain with time-travel checkout and revert buttons.
* `InsightsPage.jsx`: GitHub-style Pulse, Commits Histogram, and Contributor breakdown charts.

---

## 7. Top Viva Questions & Answers

### Q1: How does MiniVCS implement branches differently from Git?
> **Answer**: Real Git stores branch pointers as 41-byte text files in `.git/refs/heads/` pointing to a SHA-1 hash inside compressed packfiles. MiniVCS uses an **N-ary General Tree** (`BranchTree`, `TreeNode`) for logical branch relationships, combined with **Physical Directory Snapshot Isolation** (`data/branches/<branchName>/`). Each branch has an actual folder on disk containing its files, making it 100% transparent and easy to inspect directly in your file manager or terminal.

### Q2: Why is an N-ary Tree used instead of a Binary Search Tree for branches?
> **Answer**: A binary tree restricts every node to at most two children (left and right). In version control, any branch (like `main`) can have any number of child branches created off it (`dev`, `feature-1`, `feature-2`, `bugfix`). An N-ary tree stores child pointers in a dynamic `vector<TreeNode*> children`, which naturally models arbitrary branching without restrictions.

### Q3: How does the Branch Tree traversal work during branch search?
> **Answer**: Branch lookup uses **Depth-First Search (DFS)** recursion (`BranchTree::search`). It checks if the current node matches the branch name. If not, it iterates through each child in `curr->children` and recursively calls `search(child, bName)`. It runs in $O(B)$ time, where $B$ is the number of branches.

### Q4: What happens when a user switches branches?
> **Answer**: When calling `switchBranch(username, targetBranch)`:
> 1. MiniVCS clears the user's working folder `data/working/<username>/`.
> 2. MiniVCS copies all files from `data/branches/<targetBranch>/` into `data/working/<username>/`.
> 3. MiniVCS updates the user's active branch in `data/active_branches.txt`.

### Q5: Why is a LIFO Stack used for the staging area?
> **Answer**: Staging files follows a Last-In-First-Out behavior. When you stage files with `git add`, the most recently staged file is at the top of the stack. Unstaging (`git restore --staged`) naturally pops the top element first in $O(1)$ constant time.

### Q6: How does MiniVCS handle Undo and Revert on commits?
> **Answer**: Commit history is stored as a **Singly Linked List** (`CommitNode`).
> * **Undo Last Commit**: Removes the tail node from the linked list, sets `prev->next = nullptr`, restores the working directory from `parentId`, and updates `data/commits.txt`.
> * **Revert to Commit**: Finds the target commit node, deallocates all subsequent nodes in the list, sets `target->next = nullptr`, restores the target commit's snapshot, and updates `data/commits.txt`.

### Q7: Why is no third-party HTTP or JSON library used in the backend?
> **Answer**: The server is written using native POSIX sockets (`<sys/socket.h>`) and standard C++ streams (`<sstream>`). Building the socket listener and JSON serializer from scratch demonstrates mastery of network protocols, system calls, string manipulation, and low-level memory management.
