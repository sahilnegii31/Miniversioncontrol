# MiniVCS 🚀
### A Lightweight, Educational Version Control System in C++ & Modern React
> **Designed for 3rd-Semester Computer Science Students | Clean, Human-Readable & Viva-Ready**

---

## 📖 Table of Contents
1. [Project Overview & Philosophy](#1-project-overview--philosophy)
2. [Core Data Structures Explained](#2-core-data-structures-explained)
3. [Physical Folder Architecture](#3-physical-folder-architecture)
4. [Compilation & How to Run](#4-compilation--how-to-run)
5. [Detailed Code Walkthrough by File](#5-detailed-code-walkthrough-by-file)
   * [5.1 `stack.h` — LIFO Stack for Staging](#51-stackh--lifo-stack-for-staging)
   * [5.2 `commit.h` & `commit.cpp` — Singly Linked List for Commits](#52-commith--commitcpp--singly-linked-list-for-commits)
   * [5.3 `staging.h` & `staging.cpp` — Staging Management](#53-stagingh--stagingcpp--staging-management)
   * [5.4 `branch.h` & `branch.cpp` — Manual N-ary Tree & Physical Branch Isolation](#54-branchh--branchcpp--manual-n-ary-tree--physical-branch-isolation)
   * [5.5 `revert.h` & `revert.cpp` — Time-Travel, Undo & Revert](#55-reverth--revertcpp--time-travel-undo--revert)
   * [5.6 `login.h` & `login.cpp` — User Authentication](#56-loginh--logincpp--user-authentication)
   * [5.7 `contribution.h` & `contribution.cpp` — Contribution & Pulse Analytics](#57-contributionh--contributioncpp--contribution--pulse-analytics)
   * [5.8 `c_chart.h` & `c_chart.cpp` — Native C++ SVG Chart Generator](#58-c_charth--c_chartcpp--native-c-svg-chart-generator)
   * [5.9 `main.cpp` — Interactive Terminal CLI Interface](#59-maincpp--interactive-terminal-cli-interface)
   * [5.10 `server.cpp` — Embedded POSIX HTTP REST Server](#510-servercpp--embedded-posix-http-rest-server)
   * [5.11 Frontend Bridge — React Web UI](#511-frontend-bridge--react-web-ui)
6. [Top Viva Questions & Answers](#6-top-viva-questions--answers)

---

## 1. Project Overview & Philosophy

**MiniVCS** is a lightweight version control system inspired by Git, built from scratch in pure C++17 with an embedded HTTP REST server and a modern React web frontend.

Unlike Git (which uses complex cryptographic SHA packfiles and compressed blob databases), **MiniVCS is written in clean, humanized, understandable code** suitable for a 3rd-semester undergraduate student. It avoids convoluted C++ template metaprogramming, external heavy libraries, or confusing pointer tricks, and instead uses **standard, textbook Data Structures & Algorithms (DSA)** to solve real version control challenges.

### The 4 Stages of Version Control in MiniVCS:
1. **Working Directory (`data/working/<user>/`)**: Your personal desk where you create and edit code files.
2. **Staging Area (`Stack<StagedFile>`)**: A LIFO stack where files are gathered (`git add`) before being recorded permanently.
3. **Commit History (`CommitNode*` Linked List)**: A permanent, timestamped chain of snapshots (`git commit`).
4. **Physical Branch Tree (`BranchTree`, `data/branches/<branch>/`)**: An N-ary hierarchical tree where each branch has its own physical folder on disk.

---

## 2. Core Data Structures Explained

| Component | Data Structure | Why This Data Structure? |
| :--- | :--- | :--- |
| **Staging Area** | **LIFO Stack** (`Stack<T>`) | When staging files, the most recently added file is at the top of the stack. Unstaging (`git restore --staged`) naturally pops the top element first in $O(1)$ time. |
| **Commit History** | **Singly Linked List** (`CommitNode*`) | Commits happen sequentially over time. Each commit points to its parent (`parentId`), creating a chain that is easy to traverse, insert into, and revert in $O(N)$ time. |
| **Branch Hierarchy** | **Manual N-ary Tree** (`TreeNode*`, `BranchTree`) | Branches branch off parent branches (`main` $\to$ `feature` $\to$ `feature-login`). A tree with parent-child pointers is the natural mathematical model for branching. |
| **User Directory** | **Array of Structs** (`User users[]`) | Simple, bounded storage of usernames and passwords read and written to `data/users.txt`. |
| **Analytics & Heatmap** | **Hash Map / Red-Black Tree** (`std::map<string, int>`) | Groups commit counts by date (`YYYY-MM-DD`) and author in sorted order. |

---

## 3. Physical Folder Architecture

MiniVCS stores all data transparently inside the `data/` folder on your hard drive:

```text
minivcs2/
├── data/
│   ├── working/                      <-- Working directories for each user
│   │   ├── sahil/
│   │   │   └── code.c
│   │   └── vanshika/
│   ├── objects/                      <-- Permanent commit snapshots
│   │   ├── C001/                     <-- All files as they existed in Commit C001
│   │   │   └── code.c
│   │   └── C002/
│   ├── branches/                     <-- PHYSICAL BRANCH FOLDERS
│   │   ├── main/                     <-- Files on 'main' branch
│   │   │   └── code.c
│   │   └── feature/                  <-- Files on 'feature' branch
│   │       └── notes.txt
│   ├── commits.txt                   <-- Commit metadata file (ID|Author|Msg|Time|Branch|Parent)
│   ├── users.txt                     <-- User credentials (username:password)
│   └── active_branches.txt           <-- Current active branch for each user (user:branch)
```

---

## 4. Compilation & How to Run

MiniVCS uses standard `clang++` or `g++` with C++17 support:

```bash
# 1. Compile both the CLI application and the Backend Server
make all

# Or compile individually:
make minivcs    # Compiles the terminal CLI program
make server     # Compiles the REST API server
```

### Running the Terminal CLI (`minivcs`):
```bash
./minivcs
```

### Running the Full-Stack Web GUI:
```bash
# Step 1: Start the C++ Backend Server
./server 8080

# Step 2: In a separate terminal, start the React Frontend
cd frontend
npm run dev
```
Open **`http://localhost:5173`** in your browser to view the Code, Branches, and Insights tabs!

---

## 5. Detailed Code Walkthrough by File

---

### 5.1 `stack.h` — LIFO Stack for Staging
* **File Type**: Header-only C++ template class.
* **DSA Used**: Linked List-based Stack (LIFO: Last-In, First-Out).

#### Structs & Classes Defined:
1. `template <typename T> struct StackNode`
   * `T data`: Holds the payload (e.g. `StagedFile`).
   * `StackNode* next`: Pointer to the node underneath.
2. `template <typename T> class Stack`
   * `StackNode<T>* top`: Pointer to the topmost element.
   * `int count`: Tracks the number of elements.

#### Functions & Methods:

* `Stack()`
  * **Operation**: Constructor. Initializes `top = nullptr` and `count = 0`.
* `~Stack()`
  * **Operation**: Destructor. Traverses the stack and calls `delete` on each node to prevent memory leaks.
* `void push(T val)`
  * **Operation**: Pushes a new element onto the top of the stack.
  * **How it works**: Allocates a `newNode`, sets `newNode->next = top`, moves `top = newNode`, and increments `count`. ($O(1)$ time).
* `T pop()`
  * **Operation**: Removes and returns the top element of the stack.
  * **How it works**: Checks if empty. Saves `top->data`, moves `top = top->next`, frees the old top node with `delete`, and decrements `count`. ($O(1)$ time).
* `T peek()`
  * **Operation**: Returns the top element without removing it.
* `bool isEmpty()`
  * **Operation**: Returns `true` if `top == nullptr`, otherwise `false`.
* `int size()`
  * **Operation**: Returns the integer value `count`.
* `void printAll()`
  * **Operation**: Traverses from `top` to `nullptr` and prints each element to the terminal.

---

### 5.2 `commit.h` & `commit.cpp` — Singly Linked List for Commits
* **File Type**: Header and implementation file.
* **DSA Used**: Singly Linked List (`CommitNode*`).

#### Structs Defined:
`struct CommitNode`:
* `string commitId`: Unique commit hash/identifier (e.g., `C001`, `C002`).
* `string author`: Username of the committer.
* `string message`: Commit description.
* `string timestamp`: Date and time when the commit was created (`YYYY-MM-DD HH:MM:SS`).
* `string branch`: Branch where the commit belongs (e.g., `main`, `feature`).
* `string parentId`: ID of the preceding commit (`NONE` for root commit).
* `CommitNode* next`: Pointer to the next commit in the chain.

#### Functions:

* `string getCurrentTime()`
  * **Operation**: Retrieves the system clock using `<ctime>` and formats it as `YYYY-MM-DD HH:MM:SS`.
* `string generateCommitId(CommitNode* head)`
  * **Operation**: Auto-increments commit IDs (`C001`, `C002`, `C003`).
  * **How it works**: Traverses the linked list, parses the highest number after `'C'`, increments it by 1, and formats it with leading zeros using `snprintf`.
* `CommitNode* addCommit(CommitNode* head, string author, string message, string branch)`
  * **Operation**: Appends a new commit node to the end of the commit linked list.
  * **How it works**: Creates `newNode`, fills metadata, links the parent pointer to the current tail of the list, attaches `newNode` at the end, and returns the list head.
* `CommitNode* loadCommits()`
  * **Operation**: Reads `data/commits.txt` and reconstructs the commit linked list in RAM upon startup.
  * **How it works**: Opens the file, reads line-by-line, parses pipe-delimited fields (`commitId|author|msg|time|branch|parentId`), creates nodes, and links them sequentially.
* `void saveCommits(CommitNode* head)`
  * **Operation**: Serializes the commit linked list to disk in `data/commits.txt`.
  * **How it works**: Traverses `head` to `nullptr` and writes each commit's attributes separated by `|`.
* `void showHistory(CommitNode* head)`
  * **Operation**: Implements `git log` in the CLI. Prints the commit history chain to the console.
* `CommitNode* findCommit(CommitNode* head, string commitId)`
  * **Operation**: Searches the linked list for a commit with matching `commitId`. Returns pointer to node or `nullptr`.
* `void viewCommitChanges(CommitNode* head, string commitId, string currentUser)`
  * **Operation**: Implements `git diff` / `git show`. Compares files in `data/objects/<commitId>/` with its parent `data/objects/<parentId>/` and displays whether files were `[INITIAL]`, `[ADDED]`, `[MODIFIED]`, or `[UNCHANGED]`.
* `void freeCommits(CommitNode* head)`
  * **Operation**: Frees all allocated `CommitNode` memory to prevent memory leaks.

---

### 5.3 `staging.h` & `staging.cpp` — Staging Management
* **File Type**: Staging Area controller class.
* **DSA Used**: Internal `Stack<StagedFile>`.

#### Structs Defined:
`struct StagedFile`:
* `string filename`: Name of the file (e.g. `code.c`).
* `string content`: Complete text content of the file.

#### Methods in `StagingArea`:

* `void stageFile(string filename, string content)`
  * **Operation**: Implements `git add <file>`. Stages a file into the stack.
  * **How it works**: Uses a temporary stack to check if the file is already staged. If yes, it updates the content; if no, it pushes a new `StagedFile` onto `stagingStack`.
* `void unstageLastFile()`
  * **Operation**: Implements `git restore --staged`. Pops the top staged file from `stagingStack`.
* `void viewStaging()`
  * **Operation**: Implements `git status`. Prints all files currently staged waiting to be committed.
* `CommitNode* commitAll(CommitNode* head, string author, string branch)`
  * **Operation**: Prompts the user for a commit message in the CLI, then invokes `commitAllWithMessage`.
* `CommitNode* commitAllWithMessage(CommitNode* head, string author, string branch, const string& commitMsg)`
  * **Operation**: Implements `git commit -m "..."`.
  * **How it works**:
    1. First calls `syncWorkingWithBranch` so uncommitted changes in the branch are synchronized.
    2. Calls `addCommit` to add a new `CommitNode` to the linked list.
    3. Creates a permanent snapshot directory: `data/objects/<newCommitId>/`.
    4. Copies all existing files from `data/branches/<branch>/` into the snapshot directory.
    5. Pops all files from `stagingStack` and writes them into `data/objects/<newCommitId>/` (overwriting modified files).
    6. Calls `syncBranchWithCommit` to update `data/branches/<branch>/`.
    7. Copies the snapshot back into `data/working/<author>/`.
    8. Calls `saveCommits` to save changes to disk.
* `vector<StagedFile> getStagedFiles()`
  * **Operation**: Returns a vector of all currently staged files (used by the REST API server).
* `void clear()`
  * **Operation**: Pops and discards all items in the staging stack.
* `bool isEmpty()` & `int getCount()`
  * **Operation**: Helper status queries.

---

### 5.4 `branch.h` & `branch.cpp` — Manual N-ary Tree & Branch Folders
* **File Type**: Branch Management module.
* **Header / Implementation**: `branch.h` contains definitions and class structure; `branch.cpp` contains all function implementations.
* **DSA Used**: **Manual N-ary Tree** (`TreeNode` and `BranchTree`) + Physical Directory Mirroring (`data/branches/<branchName>/`).

#### Structs & Classes Defined:
1. `struct BranchDetail`:
   * Simple DTO holding branch name, parent branch, commit ID, commit message, file count, and file names list.
2. `struct TreeNode`:
   * `string name`: Branch name (e.g. `main`, `feature`).
   * `string parentBranch`: The parent branch it was branched off from.
   * `vector<TreeNode*> children`: Pointers to child branch nodes in the tree.
3. `class BranchTree`:
   * Represents the branch hierarchy as an N-ary Tree where the root node is always `main`.

#### Methods in `BranchTree`:
* `BranchTree()`: Initializes root node `main` with empty parent.
* `~BranchTree()`: Traverses and frees all tree nodes using post-order recursion.
* `TreeNode* find(string bName)`: Uses recursive Depth-First Search (`search`) to locate a branch node in the tree.
* `bool exists(string bName)`: Returns true if the branch exists in the tree.
* `void insert(string bName, string parentName = "main")`: Finds `parentName` in the tree and appends a new `TreeNode(bName, parentName)` to its `children`.
* `void displayTree()`: Prints an ASCII tree hierarchy with indentation (e.g. `main` -> `|-- feature`).

#### Branch Functions in `branch.cpp`:
* `makeDir(string path)`: Creates a directory on disk.
* `dirExists(string path)`: Checks if a directory exists using `opendir`.
* `copyFile(string src, string dst)`: Copies file content using binary streams.
* `clearDirectory(string dirPath)`: Deletes files inside a folder without deleting the folder itself.
* `copyDirectory(string srcDir, string dstDir)`: Copies all files from one directory to another.
* `vector<string> getBranchFiles(string branchName)`: Reads file names inside `data/branches/<branchName>/`.
* `string readBranchFileContent(string branchName, string filename)`: Reads file content from a branch directory into a string.
* `void ensureBranchDir(string branchName)`: Ensures `data/branches/<branchName>` folder exists on disk.
* `void syncWorkingWithBranch(string username, string branchName)`:
  * Syncs files between the user's working folder and the active branch folder.
  * If a branch has a file not yet in working folder, copies branch -> working.
  * Copies working files into the branch folder so changes reflect across both.
* `string getUserCurrentBranch(string username)` & `setUserCurrentBranch(...)`:
  * Reads/writes the active branch per user in `data/active_branches.txt` (defaults to `"main"`).
* `vector<string> listAllBranches()`:
  * Reads subfolders inside `data/branches/` to discover all existing branches.
* `void syncBranchWithCommit(string branchName, string commitId)`:
  * Clears and copies files from the commit snapshot `data/objects/<commitId>/` to `data/branches/<branchName>/`.
* `void initBranchSystem(CommitNode* head)`:
  * Seeds the `main` branch folder, restores tree from commit history, and copies initial files if needed.
* `vector<BranchDetail> getAllBranchesDetailed(CommitNode* head)`:
  * Gathers full branch details (name, parent, latest commit, files) for CLI display and REST API.
* `bool createBranch(string newBranch, string fromBranch, string creator)`:
  * Validates branch name, creates folder `data/branches/<newBranch>/`, copies files from parent branch, and inserts into `BranchTree`.
* `bool switchBranch(string username, string targetBranch)`:
  * Clears `data/working/<username>/` and loads files from `data/branches/<targetBranch>/`. Updates user's active branch.
* `bool removeBranch(string branchName, string& errorMsg)`:
  * Deletes a branch folder (safeguarded against deleting `main`).

---

### 5.5 `revert.h` & `revert.cpp` — Time-Travel, Undo & Revert
* **File Type**: Historical time-travel engine.
* **DSA Used**: Singly Linked List deletion and truncation.

#### Core Engine Functions:
* `bool restoreSnapshot(string commitId, string username)`:
  * Copies snapshot files from `data/objects/<commitId>/` into `data/working/<username>/`.
* `bool checkoutCommitDirect(CommitNode* head, string commitId, string currentUser)`:
  * Non-interactive checkout of a past commit snapshot used by REST API.
* `CommitNode* undoLastCommitDirect(CommitNode* head, string currentUser)`:
  * Removes the tail commit node from the linked list, restores the parent commit snapshot, and saves `data/commits.txt`.
* `CommitNode* revertToCommitDirect(CommitNode* head, string targetId, string currentUser)`:
  * Truncates all commit nodes after `targetId`, sets `target->next = nullptr`, restores files from `targetId`, and updates `data/commits.txt`.

#### Interactive CLI Functions:
* `bool checkoutCommit(CommitNode* head, string commitId, string currentUser)`:
  * Prompts for user confirmation in CLI and delegates to `checkoutCommitDirect`.
* `CommitNode* undoLastCommit(CommitNode* head, string currentUser)`:
  * Prompts for user confirmation in CLI and delegates to `undoLastCommitDirect`.
* `CommitNode* revertToCommit(CommitNode* head, string targetId, string currentUser)`:
  * Prompts for user confirmation in CLI and delegates to `revertToCommitDirect`.

---

### 5.6 `login.h` & `login.cpp` — User Authentication
* **File Type**: User login and registration manager.
* **DSA Used**: Fixed array of `User` structs (`User users[MAX_USERS]`).

#### Functions:
* `int loadUsers(User users[], int maxUsers)`
  * **Operation**: Reads `data/users.txt` and loads `username:password` into memory. Returns total user count.
* `void saveUsers(User users[], int count)`
  * **Operation**: Writes `users[]` array back to `data/users.txt`.
* `bool registerUser(User users[], int &count)`
  * **Operation**: CLI registration prompt. Checks for duplicate usernames, adds user to array, and calls `saveUsers`.
* `string loginUser(User users[], int count)`
  * **Operation**: CLI login prompt. Matches username and password. Returns logged-in username.
* `string runLoginMenu()`
  * **Operation**: CLI entry point loop giving options to Login, Register, or Exit.
* `loginUserDirect` & `registerUserDirect`
  * **Operation**: Direct programmatic functions called by the REST API server for web authentication.

---

### 5.7 `contribution.h` & `contribution.cpp` — Contribution & Pulse Analytics
* **File Type**: GitHub-style Insights metrics calculator.
* **DSA Used**: `std::map<string, int>` for date-based and author-based frequency aggregation.

#### Structs:
* `DayActivity`: Stores `{ string date, int count }`.
* `AuthorStats`: Stores `{ string author, int commitCount, int percentage }`.
* `ContributionSummary`: Contains metrics (`totalCommits`, `activeDays`, `currentStreak`, `longestStreak`, `authorBreakdown`).

#### Functions:
* `ContributionSummary calculateContributions(CommitNode* head, const string& authorFilter)`
  * **Operation**: Scans the commit linked list, aggregates commit volume over the last 14 days, computes streak metrics, and calculates contributor percentages.
* `string generateContributionSvg(const ContributionSummary& summary)`
  * **Operation**: Calls the `cchart` library to generate a pure SVG commit histogram.
* `string generateContributorSvg(const ContributionSummary& summary)`
  * **Operation**: Calls the `cchart` library to generate a contributor breakdown SVG chart.

---

### 5.8 `c_chart.h` & `c_chart.cpp` — Native C++ SVG Chart Generator
* **File Type**: Standalone vector graphics generator.
* **DSA Used**: Vector math & string stream XML assembly.

#### Class `CommitChartGenerator`:
* `void addDailyData(const string& date, int count)`: Appends day data to internal vector.
* `void addContributor(const string& author, int count, int percentage)`: Appends contributor stat.
* `string renderCommitHistogramSvg() const`:
  * Computes the maximum commit count.
  * Generates standard `<svg>`, `<rect>`, `<line>`, and `<text>` XML elements.
  * Dynamically calculates bar height and returns valid SVG vector graphics directly to the browser.
* `string renderContributorChartSvg() const`:
  * Generates horizontal progress bars for each contributor's percentage share.

---

### 5.9 `main.cpp` — Interactive Terminal CLI Interface
* **File Type**: Main terminal user interface.

#### Functions:
* `void ensureWorkingDir(string username)`: Creates `data/working/<username>/`.
* `void listBranchesMenu(CommitNode* head, const string& currentBranch)`:
  * Lists all branches with their folder paths, file counts, and commits.
  * Calls `getGlobalBranchTree().displayTree()` to render the ASCII Branch Tree.
* `void createBranchMenu(string username, string& currentBranch)`: Prompts for new branch name and source branch.
* `void switchBranchMenu(string username, string& currentBranch)`: Prompts for target branch and switches working directory.
* `void writeFileMenu(string username, const string& currentBranch)`: Prompts user to create or edit a file, saving it in both working directory and branch directory.
* `void addFileToStaging(...)`: Syncs working directory with branch and stages file into `StagingArea`.
* `void mainMenu(string username)`: The master menu loop with categorized sections:
  * `[ WORKING DIRECTORY & STAGING ]`: Write File, Stage File, View Staging, Unstage.
  * `[ COMMITS & HISTORY ]`: Commit, Commit History, Commit Diff, Checkout, Undo, Revert.
  * `[ BRANCH MANAGEMENT ]`: View Branches & Tree, Create Branch, Switch Branch.
* `int main()`: Welcomes the user, runs `runLoginMenu()`, and starts `mainMenu()`.

---

### 5.10 `server.cpp` — Embedded POSIX HTTP REST Server
* **File Type**: Zero-dependency TCP Socket Server translating HTTP requests into C++ function calls.
* **DSA & System Calls Used**: POSIX Sockets (`<sys/socket.h>`, `<netinet/in.h>`, `<unistd.h>`), Mutex (`std::mutex`), String Streams.

#### How It Works:
1. `socket(AF_INET, SOCK_STREAM, 0)`: Creates an IPv4 TCP stream socket.
2. `setsockopt(..., SO_REUSEADDR, ...)`: Allows instant socket rebinding upon restart.
3. `bind()`: Binds the socket to port `8080`.
4. `listen()`: Starts listening for incoming browser requests.
5. `accept()`: Accepts a connection from Vite/React frontend.
6. Reads HTTP method and path (`GET /api/branches`, `POST /api/commit`, etc.).
7. Dispatches request to the appropriate C++ engine function.
8. `sendHttpResponse()`: Returns JSON or SVG with `Content-Type: application/json` and `Cache-Control: no-cache`.

#### Core REST Routes:
| Method | Endpoint | Description |
| :--- | :--- | :--- |
| `GET` | `/api/status` | Server health check |
| `GET` | `/api/branches?user=<user>` | Lists all branches and active branch |
| `POST` | `/api/branch` | Creates a new branch snapshot folder |
| `POST` | `/api/branch/switch` | Switches user's checked-out branch |
| `GET` | `/api/branch/files?branch=<name>` | Lists files in a branch folder |
| `GET` | `/api/branch/file?branch=<name>&name=<f>` | Reads file content directly from branch folder |
| `DELETE` | `/api/branch?name=<name>` | Deletes non-main branch folder |
| `GET` | `/api/files?user=<user>` | Lists files in user's working directory |
| `POST` | `/api/file` | Saves file to working directory & branch folder |
| `DELETE` | `/api/file` | Deletes file from working directory & branch folder |
| `GET` | `/api/staging?user=<user>` | Lists currently staged files |
| `POST` | `/api/staging/add` | Stages a file (`git add`) |
| `POST` | `/api/staging/unstage` | Pops last file from staging stack |
| `POST` | `/api/staging/clear` | Clears staging stack |
| `POST` | `/api/commit` | Commits staged changes & synchronizes branch |
| `GET` | `/api/commits` | Returns entire commit graph |
| `GET` | `/api/commit/diff?commitId=<id>` | Returns commit diff against parent |
| `POST` | `/api/checkout` | Restores working directory to specific commit |
| `POST` | `/api/undo` | Undoes last commit |
| `POST` | `/api/revert` | Reverts commit chain |
| `GET` | `/api/contributions?user=<user>` | Returns JSON analytics with embedded SVG |

---

### 5.11 Frontend Bridge — React Web UI
The React frontend (in `frontend/src/`) communicates directly with `server.cpp`:
* **`api.js`**: Central API wrapper with `cache: 'no-store'` preventing stale file caching.
* **`Navbar.jsx`**: Top navigation with tab switcher (**Code**, **Branches**, **Insights**) and live branch switcher dropdown.
* **`WorkingDirExplorer.jsx`**: Working directory file explorer with file editor and "Stage" buttons.
* **`StagingPanel.jsx`**: Displays staged files and provides commit form with branch selector.
* **`CommitHistory.jsx`**: Shows chronological commit chain with Diff inspection modal, Undo, and Checkout buttons.
* **`BranchesPage.jsx`**:
  * Default Branch card (`main`).
  * Feature Branches grid.
  * New Branch modal.
  * **Branch File Inspector Modal**: Preview files inside any branch folder without switching branches.
* **`InsightsPage.jsx`**: GitHub-style Pulse summary, Native C++ SVG Commit Activity histogram, and Contributor breakdown.

---

## 6. Top Viva Questions & Answers

#### Q1: What data structure is used for the Staging Area, and why?
> **Answer**: A **LIFO Stack** (`Stack<T>`). When developers stage files, the latest staged file is placed at the top. When unstaging (`git restore --staged`), the last added file is popped first, making the Stack the most optimal and natural data structure ($O(1)$ push and pop).

#### Q2: How does MiniVCS store and navigate commit history?
> **Answer**: Using a **Singly Linked List** (`CommitNode*`). Each commit stores its metadata (`commitId`, `author`, `message`, `timestamp`, `branch`) and points to its parent (`parentId`). Navigating commit history is a simple linked list traversal.

#### Q3: How is branching implemented in MiniVCS?
> **Answer**: MiniVCS uses a **Manual N-ary Tree** (`BranchTree`, `TreeNode`) for logical branch relationships, paired with **Physical Directory Snapshot Isolation** (`data/branches/<branchName>/`). Each branch has its own physical folder containing its snapshot. Switching branches syncs the user's working directory with the branch folder.

#### Q4: How did you fix the bug where files edited directly in VS Code weren't updating in the branch?
> **Answer**: We implemented **Bi-directional Synchronization** (`syncWorkingWithBranch`). It compares file modification timestamps (`st_mtime`) between `data/branches/<branch>/` and `data/working/<user>/`. Any edits made directly in VS Code inside `data/branches/` are instantly synchronized to the working directory before staging or committing, and browser caching was disabled using `Cache-Control: no-cache`.

#### Q5: How does MiniVCS generate charts without external JavaScript chart libraries?
> **Answer**: The charts are generated natively in **C++** (`c_chart.cpp`). C++ calculates bar heights via linear normalization and formats pure SVG XML strings (`<svg>`, `<rect>`, `<text>`), which are streamed over HTTP to the React frontend.
