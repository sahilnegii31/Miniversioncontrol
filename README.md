# MiniVCS - Mini Version Control System

MiniVCS is a lightweight version control system implemented in C++ designed to emulate fundamental Git concepts such as file staging, automated commit snapshots, history inspection, viewing differences between commits, and commit rollback (undo, revert, and checkout).

The project strictly complies with academic constraints prohibiting predefined STL container classes such as `std::stack`, `std::queue`, `std::map`, and `std::unordered_map`. Core data structures are implemented from scratch using linked lists, structs, and arrays.

---

## Directory Architecture

```
minivcs2/
├── main.cpp          # Menu-driven CLI entry point
├── stack.h           # Custom generic Stack implemented via singly linked list
├── login.h           # Authentication declarations and User data structure
├── login.cpp         # User registration and login implementation
├── commit.h          # Commit linked list data structures and declarations
├── commit.cpp        # Commit creation, serialization, diff/change inspection
├── staging.h         # Staging area class declarations
├── staging.cpp       # Stack-based staging area implementation and snapshot creation
├── revert.h          # Undo, Revert, and Checkout declarations
├── revert.cpp        # History rollback and snapshot restoration
├── Makefile          # Build configuration
└── README.md         # Comprehensive project documentation

data/                 # Created automatically at runtime
├── users.txt         # User credentials (username:password)
├── commits.txt       # Commit metadata records
├── objects/          # Commit snapshots
│   ├── C001/         # Snapshot files for commit C001
│   │   └── notes.txt
│   └── C002/         # Snapshot files for commit C002
└── working/          # User working directories
    ├── alice/        # Working workspace for user 'alice'
    └── bob/          # Working workspace for user 'bob'
```

---

## Data Structures & Implementation Details

| Module | Data Structure | STL Used? | Description |
|---|---|---|---|
| **Custom Stack** | Singly Linked List | None | Generic LIFO stack using template nodes with head insertion and deletion |
| **Authentication** | Fixed-size array of `User` structs | None | Linear search for login and uniqueness validation |
| **Commit History** | Singly Linked List | None | Sequential node chain storing commit metadata with parent pointers |
| **Staging Area** | Custom `Stack<StagedFile>` | None | LIFO buffer holding modified files before snapshot creation |
| **Revert & Undo** | Singly Linked List Traversal | None | Pointer rewiring and directory file copying for state restoration |

---

## Modules and Functions

### 1. Custom Stack (`stack.h`)

Generic template class providing fundamental LIFO operations without standard library containers.

* **`StackNode<T>`**: Node struct storing `T data` and `StackNode* next`.
* **`void push(T val)`**: Inserts a new element at the top of the stack ($O(1)$).
* **`T pop()`**: Removes and returns the element at the top of the stack ($O(1)$).
* **`T peek()`**: Returns the top element without removing it ($O(1)$).
* **`bool isEmpty()`**: Returns `true` if the stack contains no elements.
* **`int size()`**: Returns the total count of elements currently stored.
* **`void printAll()`**: Traverses from top to bottom and prints elements.
* **`~Stack()`**: Destructor that deallocates all linked nodes.

---

### 2. User Authentication (`login.h`, `login.cpp`)

Handles user credential persistence and session management.

* **`struct User`**: Holds `char username[50]` and `char password[50]`.
* **`void ensureDataFolder()`**: Ensures the `data/` directory exists.
* **`int loadUsers(User users[], int maxUsers)`**: Reads `data/users.txt` and populates the user array.
* **`void saveUsers(User users[], int count)`**: Writes all user records back to `data/users.txt`.
* **`bool registerUser(User users[], int &count)`**: Prompts for credentials, verifies username uniqueness, and appends the new account.
* **`string loginUser(User users[], int count)`**: Validates username and password; returns the authenticated username upon success.
* **`string runLoginMenu()`**: Displays the authentication menu loop (Login / Register / Exit) until an authenticated session is established.

---

### 3. Commit Management (`commit.h`, `commit.cpp`)

Manages version history, parent-child commit relationships, persistence, and change inspection.

* **`struct CommitNode`**: Contains:
  * `string commitId`: Formatted identifier (e.g., `C001`, `C002`).
  * `string author`: Username of the committer.
  * `string message`: Description of the commit.
  * `string timestamp`: Human-readable date and time.
  * `string branch`: Branch name (defaults to `main`).
  * `string parentId`: Identifier of the immediate predecessor commit (`NONE` for root).
  * `CommitNode* next`: Pointer to the next commit in the chain.
* **`string getCurrentTime()`**: Returns current local timestamp formatted as `YYYY-MM-DD HH:MM:SS`.
* **`string generateCommitId(CommitNode* head)`**: Computes the highest commit number in the chain and increments it, returning a zero-padded string (`C%03d`).
* **`CommitNode* addCommit(CommitNode* head, string author, string message, string branch)`**: Appends a new commit node to the tail of the linked list.
* **`CommitNode* loadCommits()`**: Parses `data/commits.txt` and reconstructs the commit linked list into memory.
* **`void saveCommits(CommitNode* head)`**: Serializes the linked list of commits into `data/commits.txt` using pipe-delimited format:
  `commitId|author|message|timestamp|branch|parentId`.
* **`void showHistory(CommitNode* head)`**: Traverses and displays all commits with full metadata.
* **`CommitNode* findCommit(CommitNode* head, string commitId)`**: Performs linear search to locate a commit node by its unique ID.
* **`void viewCommitChanges(CommitNode* head, string commitId, string currentUser)`**:
  * Inspects a specific commit snapshot (`data/objects/<commitId>/`).
  * Displays file contents for that commit.
  * Compares each file with the parent commit snapshot (`data/objects/<parentId>/`).
  * Categorizes and displays status: `[INITIAL COMMIT]`, `[ADDED]`, `[MODIFIED]`, `[UNCHANGED]`, or `[DELETED]`.
  * Shows both previous and current file contents for modified files.
  * Prompts whether the user wants to checkout/move working directory to this commit.
* **`void freeCommits(CommitNode* head)`**: Frees heap memory allocated for all commit nodes upon program termination.

---

### 4. Staging Area (`staging.h`, `staging.cpp`)

Emulates the staging environment (`git add`) prior to creating a permanent snapshot.

* **`struct StagedFile`**: Holds `string filename` and `string content`.
* **`void stageFile(string filename, string content)`**: Adds a file to the staging stack. If a file with the same name is already staged, updates its content.
* **`void unstageLastFile()`**: Removes the most recently staged file from the stack (`pop`).
* **`void viewStaging()`**: Traverses the staging stack and prints all currently staged file names and contents.
* **`CommitNode* commitAll(CommitNode* head, string author, string branch)`**:
  1. Requests a commit message from the user.
  2. Generates a new commit node and attaches it to the commit history.
  3. Creates a new snapshot folder under `data/objects/<commitId>/`.
  4. Flushes each file from the staging stack into the snapshot directory.
  5. Updates `data/commits.txt` and resets the staging stack.
* **`bool isEmpty()`**: Checks whether the staging stack is empty.
* **`int getCount()`**: Returns the count of files staged.

---

### 5. Revert, Checkout & Undo Operations (`revert.h`, `revert.cpp`)

Provides version inspection, checkout, and rollback mechanisms.

* **`bool checkoutCommit(CommitNode* head, string commitId, string currentUser)`**:
  * Validates the commit ID.
  * Restores that commit snapshot into `data/working/<username>/` without deleting subsequent commit history.
* **`CommitNode* undoLastCommit(CommitNode* head, string currentUser)`**:
  * Deletes the most recent commit node (tail of the linked list).
  * Updates `data/commits.txt`.
  * Restores the parent commit snapshot directly into the user's working directory (`data/working/<username>/`).
* **`CommitNode* revertToCommit(CommitNode* head, string targetId, string currentUser)`**:
  * Locates `targetId` in the commit chain.
  * Deletes all commit nodes following `targetId`.
  * Restores the target commit snapshot into the user's working directory.
  * Updates persistent commit records.
* **`bool restoreSnapshot(string commitId, string username)`**:
  * Reads snapshot files from `data/objects/<commitId>/` using POSIX directory traversal (`opendir`/`readdir`).
  * Copies each snapshot file into `data/working/<username>/`.

---

### 6. Main Interface (`main.cpp`)

Provides a clean, interactive menu-driven interface:

1. **Write File**: Creates or edits a file inside the authenticated user's working directory.
2. **Add File to Staging**: Reads a specified file from the working directory and stages it.
3. **View Staging Area**: Lists all files ready to be committed.
4. **Unstage Last File**: Removes the top file from the staging area.
5. **Commit Staged Files**: Converts staged files into an immutable snapshot directory under `data/objects/<commitId>/`.
6. **View Commit History**: Displays sequential commit history.
7. **View Commit Details & File Changes**: Displays commit snapshot files, contents, and changes made relative to its parent commit.
8. **Checkout / Move to Specific Commit**: Restores working directory files to match any chosen commit.
9. **Undo Last Commit**: Rolls back the latest commit to its immediate parent.
10. **Revert to Specific Commit**: Rolls back history to a specified commit ID.
0. **Logout**: Exits the user session.

---

## Compilation and Execution

The project compiles with standard C++11 and POSIX system calls:

```bash
cd minivcs2

# Compile
make

# Execute
./minivcs

# Or compile and run directly
make run

# Clean binary and runtime data
make clean
```

---

## Team Module Distribution

| Member | Assigned Modules | Source Files |
|---|---|---|
| **Member 1** | Staging Area & Data Structure | `staging.h`, `staging.cpp`, `stack.h` |
| **Member 2** | Undo, Revert, Checkout & Snapshot Restoration | `revert.h`, `revert.cpp` |
| **Member 3** | Authentication & Commit Engine / Diff Viewer | `login.h`, `login.cpp`, `commit.h`, `commit.cpp` |
| **Member 4** | CLI System & Build Configuration | `main.cpp`, `Makefile`, `README.md` |
