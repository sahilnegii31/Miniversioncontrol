#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "commit.h"
#include "staging.h"
#include "revert.h"
#include "login.h"
#include "contribution.h"
#include "branch.h"

using namespace std;

static const int PORT = 8080;
static mutex vcsMutex;
static CommitNode* globalCommitHead = nullptr;
static map<string, StagingArea> userStagingMap;

// Helper to reload commits from data/commits.txt
// Ensures CLI (main.cpp) commits and Web GUI commits stay 100% in sync
static void syncCommitsFromDisk() {
    if (globalCommitHead != nullptr) {
        freeCommits(globalCommitHead);
        globalCommitHead = nullptr;
    }
    globalCommitHead = loadCommits();
}

// Helper: JSON string escape
static string jsonEscape(const string& s) {
    ostringstream oss;
    for (char c : s) {
        switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b";  break;
            case '\f': oss << "\\f";  break;
            case '\n': oss << "\\n";  break;
            case '\r': oss << "\\r";  break;
            case '\t': oss << "\\t";  break;
            default:
                if ('\x00' <= c && c <= '\x1f') {
                    oss << "\\u" << hex << uppercase << setw(4) << setfill('0') << (int)c;
                } else {
                    oss << c;
                }
        }
    }
    return oss.str();
}

// Simple JSON field extractor for "key": "value" or "key": value
static string extractJsonField(const string& json, const string& key) {
    string pattern = "\"" + key + "\"";
    size_t pos = json.find(pattern);
    if (pos == string::npos) return "";

    pos = json.find(':', pos + pattern.size());
    if (pos == string::npos) return "";
    pos++;

    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\r' || json[pos] == '\n')) {
        pos++;
    }

    if (pos >= json.size()) return "";

    if (json[pos] == '"') {
        pos++;
        string val = "";
        while (pos < json.size() && json[pos] != '"') {
            if (json[pos] == '\\' && pos + 1 < json.size()) {
                pos++;
                if (json[pos] == 'n') val += '\n';
                else if (json[pos] == 'r') val += '\r';
                else if (json[pos] == 't') val += '\t';
                else if (json[pos] == '"') val += '"';
                else if (json[pos] == '\\') val += '\\';
                else val += json[pos];
            } else {
                val += json[pos];
            }
            pos++;
        }
        return val;
    } else {
        string val = "";
        while (pos < json.size() && json[pos] != ',' && json[pos] != '}' && json[pos] != ' ' && json[pos] != '\r' && json[pos] != '\n') {
            val += json[pos++];
        }
        return val;
    }
}

// URL query parameter extractor
static string getQueryParam(const string& path, const string& param) {
    size_t qPos = path.find('?');
    if (qPos == string::npos) return "";
    string query = path.substr(qPos + 1);

    string search = param + "=";
    size_t pos = query.find(search);
    if (pos == string::npos) return "";

    size_t start = pos + search.size();
    size_t end = query.find('&', start);
    string val = (end == string::npos) ? query.substr(start) : query.substr(start, end - start);

    // URL decode %20 etc.
    string decoded = "";
    for (size_t i = 0; i < val.size(); i++) {
        if (val[i] == '%' && i + 2 < val.size()) {
            string hexStr = val.substr(i + 1, 2);
            char c = (char)strtol(hexStr.c_str(), nullptr, 16);
            decoded += c;
            i += 2;
        } else if (val[i] == '+') {
            decoded += ' ';
        } else {
            decoded += val[i];
        }
    }
    return decoded;
}

static void sendHttpResponse(int clientSock, int statusCode, const string& statusText,
                             const string& contentType, const string& body) {
    ostringstream oss;
    oss << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    oss << "Content-Type: " << contentType << "\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Access-Control-Allow-Origin: *\r\n";
    oss << "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n";
    oss << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    oss << "Cache-Control: no-cache, no-store, must-revalidate\r\n";
    oss << "Pragma: no-cache\r\n";
    oss << "Expires: 0\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";
    oss << body;

    string resp = oss.str();
    send(clientSock, resp.c_str(), resp.size(), 0);
}

static void sendCorsPreflight(int clientSock) {
    string header = "HTTP/1.1 204 No Content\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n"
                    "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: close\r\n\r\n";
    send(clientSock, header.c_str(), header.size(), 0);
}

// Convert commit list to JSON string
static string commitsToJson(CommitNode* head) {
    ostringstream oss;
    oss << "[\n";
    CommitNode* curr = head;
    bool first = true;
    while (curr != nullptr) {
        if (!first) oss << ",\n";
        first = false;
        oss << "  {\n";
        oss << "    \"commitId\": \"" << jsonEscape(curr->commitId) << "\",\n";
        oss << "    \"author\": \"" << jsonEscape(curr->author) << "\",\n";
        oss << "    \"message\": \"" << jsonEscape(curr->message) << "\",\n";
        oss << "    \"timestamp\": \"" << jsonEscape(curr->timestamp) << "\",\n";
        oss << "    \"branch\": \"" << jsonEscape(curr->branch) << "\",\n";
        oss << "    \"parentId\": \"" << jsonEscape(curr->parentId) << "\"\n";
        oss << "  }";
        curr = curr->next;
    }
    oss << "\n]";
    return oss.str();
}

static void ensureUserDir(const string& user) {
    mkdir("data", 0755);
    mkdir("data/working", 0755);
    string userDir = "data/working/" + user;
    mkdir(userDir.c_str(), 0755);
}

static void handleClient(int clientSock) {
    char buffer[65536];
    int bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        close(clientSock);
        return;
    }
    buffer[bytesRead] = '\0';
    string rawRequest(buffer, bytesRead);

    // Extract method and path
    istringstream reqStream(rawRequest);
    string method, path, protocol;
    reqStream >> method >> path >> protocol;

    // Handle OPTIONS CORS preflight
    if (method == "OPTIONS") {
        sendCorsPreflight(clientSock);
        close(clientSock);
        return;
    }

    // Extract request body and ensure full read based on Content-Length
    size_t bodyPos = rawRequest.find("\r\n\r\n");
    string reqBody = "";
    if (bodyPos != string::npos) {
        reqBody = rawRequest.substr(bodyPos + 4);
    }

    size_t clPos = rawRequest.find("Content-Length: ");
    if (clPos == string::npos) clPos = rawRequest.find("content-length: ");
    if (clPos != string::npos) {
        size_t clEnd = rawRequest.find("\r\n", clPos);
        if (clEnd != string::npos) {
            int cl = atoi(rawRequest.substr(clPos + 16, clEnd - (clPos + 16)).c_str());
            while ((int)reqBody.size() < cl) {
                int more = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
                if (more <= 0) break;
                reqBody.append(buffer, more);
            }
        }
    }

    string basePath = path;
    size_t qPos = path.find('?');
    if (qPos != string::npos) {
        basePath = path.substr(0, qPos);
    }

    lock_guard<mutex> lock(vcsMutex);

    // Endpoint: /api/status
    if (basePath == "/api/status" && method == "GET") {
        sendHttpResponse(clientSock, 200, "OK", "application/json",
                         "{\"status\": \"ok\", \"server\": \"MiniVCS C++ Core\"}");
    }
    // Endpoint: /api/login
    else if (basePath == "/api/login" && method == "POST") {
        string uname = extractJsonField(reqBody, "username");
        string pass  = extractJsonField(reqBody, "password");

        User users[MAX_USERS];
        int count = loadUsers(users, MAX_USERS);
        bool ok = loginUserDirect(users, count, uname, pass);

        if (ok) {
            ensureUserDir(uname);
            sendHttpResponse(clientSock, 200, "OK", "application/json",
                             "{\"success\": true, \"username\": \"" + jsonEscape(uname) + "\"}");
        } else {
            sendHttpResponse(clientSock, 401, "Unauthorized", "application/json",
                             "{\"success\": false, \"error\": \"Invalid username or password\"}");
        }
    }
    // Endpoint: /api/register
    else if (basePath == "/api/register" && method == "POST") {
        string uname = extractJsonField(reqBody, "username");
        string pass  = extractJsonField(reqBody, "password");

        User users[MAX_USERS];
        int count = loadUsers(users, MAX_USERS);
        string errMsg = "";
        bool ok = registerUserDirect(users, count, uname, pass, errMsg);

        if (ok) {
            ensureUserDir(uname);
            sendHttpResponse(clientSock, 200, "OK", "application/json",
                             "{\"success\": true, \"username\": \"" + jsonEscape(uname) + "\"}");
        } else {
            sendHttpResponse(clientSock, 400, "Bad Request", "application/json",
                             "{\"success\": false, \"error\": \"" + jsonEscape(errMsg) + "\"}");
        }
    }
    // Endpoint: /api/users
    else if (basePath == "/api/users" && method == "GET") {
        User users[MAX_USERS];
        int count = loadUsers(users, MAX_USERS);

        ostringstream oss;
        oss << "[\n";
        for (int i = 0; i < count; i++) {
            oss << "  {\"username\": \"" << jsonEscape(users[i].username) << "\"}";
            if (i + 1 < count) oss << ",";
            oss << "\n";
        }
        oss << "]";
        sendHttpResponse(clientSock, 200, "OK", "application/json", oss.str());
    }
    // Endpoint: /api/files (list working directory files)
    else if (basePath == "/api/files" && method == "GET") {
        string user = getQueryParam(path, "user");
        if (user.empty()) user = "sahil";
        ensureUserDir(user);

        string activeBranch = getUserCurrentBranch(user);
        syncWorkingWithBranch(user, activeBranch);

        string userDir = "data/working/" + user;
        DIR* dir = opendir(userDir.c_str());
        ostringstream oss;
        oss << "[\n";
        bool first = true;

        if (dir != nullptr) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                string fname = entry->d_name;
                if (fname == "." || fname == "..") continue;

                string fullPath = userDir + "/" + fname;
                struct stat st;
                long size = 0;
                if (stat(fullPath.c_str(), &st) == 0) {
                    size = st.st_size;
                }

                if (!first) oss << ",\n";
                first = false;
                oss << "  {\"name\": \"" << jsonEscape(fname)
                    << "\", \"size\": " << size << "}";
            }
            closedir(dir);
        }
        oss << "\n]";
        sendHttpResponse(clientSock, 200, "OK", "application/json", oss.str());
    }
    // Endpoint: /api/file (GET content)
    else if (basePath == "/api/file" && method == "GET") {
        string user = getQueryParam(path, "user");
        string fname = getQueryParam(path, "name");
        if (user.empty()) user = "sahil";

        string activeBranch = getUserCurrentBranch(user);
        syncWorkingWithBranch(user, activeBranch);

        string filePath = "data/working/" + user + "/" + fname;
        ifstream f(filePath.c_str());
        if (!f.is_open()) {
            sendHttpResponse(clientSock, 404, "Not Found", "application/json",
                             "{\"error\": \"File not found\"}");
        } else {
            string content = "", line;
            while (getline(f, line)) {
                content += line + "\n";
            }
            f.close();
            ostringstream oss;
            oss << "{\"name\": \"" << jsonEscape(fname)
                << "\", \"content\": \"" << jsonEscape(content) << "\"}";
            sendHttpResponse(clientSock, 200, "OK", "application/json", oss.str());
        }
    }
    // Endpoint: /api/file (POST save/update file)
    else if (basePath == "/api/file" && method == "POST") {
        string user    = extractJsonField(reqBody, "user");
        string fname   = extractJsonField(reqBody, "name");
        string content = extractJsonField(reqBody, "content");
        if (user.empty()) user = "sahil";

        if (fname.empty()) {
            sendHttpResponse(clientSock, 400, "Bad Request", "application/json",
                             "{\"success\": false, \"error\": \"File name cannot be empty\"}");
        } else {
            ensureUserDir(user);
            string filePath = "data/working/" + user + "/" + fname;
            ofstream out(filePath.c_str());
            if (out.is_open()) {
                out << content;
                out.close();

                string activeBranch = getUserCurrentBranch(user);
                ensureBranchDir(activeBranch);
                string branchFilePath = "data/branches/" + activeBranch + "/" + fname;
                ofstream bOut(branchFilePath.c_str());
                if (bOut.is_open()) {
                    bOut << content;
                    bOut.close();
                }

                sendHttpResponse(clientSock, 200, "OK", "application/json",
                                 "{\"success\": true, \"name\": \"" + jsonEscape(fname) + "\"}");
            } else {
                sendHttpResponse(clientSock, 500, "Internal Server Error", "application/json",
                                 "{\"success\": false, \"error\": \"Could not write file: " + jsonEscape(filePath) + "\"}");
            }
        }
    }
    // Endpoint: /api/file (DELETE file)
    else if (basePath == "/api/file" && method == "DELETE") {
        string user = getQueryParam(path, "user");
        string fname = getQueryParam(path, "name");
        if (user.empty()) user = "sahil";

        string filePath = "data/working/" + user + "/" + fname;
        remove(filePath.c_str());

        string activeBranch = getUserCurrentBranch(user);
        string branchFilePath = "data/branches/" + activeBranch + "/" + fname;
        remove(branchFilePath.c_str());

        sendHttpResponse(clientSock, 200, "OK", "application/json",
                         "{\"success\": true}");
    }
    // Endpoint: /api/staging (GET staged files)
    else if (basePath == "/api/staging" && method == "GET") {
        string user = getQueryParam(path, "user");
        if (user.empty()) user = "sahil";

        vector<StagedFile> staged = userStagingMap[user].getStagedFiles();
        ostringstream oss;
        oss << "[\n";
        for (size_t i = 0; i < staged.size(); i++) {
            oss << "  {\"filename\": \"" << jsonEscape(staged[i].filename)
                << "\", \"content\": \"" << jsonEscape(staged[i].content) << "\"}";
            if (i + 1 < staged.size()) oss << ",";
            oss << "\n";
        }
        oss << "]";
        sendHttpResponse(clientSock, 200, "OK", "application/json", oss.str());
    }
    // Endpoint: /api/staging/add (Stage a file)
    else if (basePath == "/api/staging/add" && method == "POST") {
        string user  = extractJsonField(reqBody, "user");
        string fname = extractJsonField(reqBody, "filename");
        if (user.empty()) user = "sahil";

        string activeBranch = getUserCurrentBranch(user);
        syncWorkingWithBranch(user, activeBranch);

        string filePath = "data/working/" + user + "/" + fname;
        ifstream f(filePath.c_str());
        if (!f.is_open()) {
            sendHttpResponse(clientSock, 404, "Not Found", "application/json",
                             "{\"success\": false, \"error\": \"File not found in working directory\"}");
        } else {
            string content = "", line;
            while (getline(f, line)) {
                content += line + "\n";
            }
            f.close();

            userStagingMap[user].stageFile(fname, content);
            sendHttpResponse(clientSock, 200, "OK", "application/json",
                             "{\"success\": true, \"message\": \"File staged successfully\"}");
        }
    }
    // Endpoint: /api/staging/unstage (Unstage last file)
    else if (basePath == "/api/staging/unstage" && method == "POST") {
        string user = extractJsonField(reqBody, "user");
        if (user.empty()) user = "sahil";

        userStagingMap[user].unstageLastFile();
        sendHttpResponse(clientSock, 200, "OK", "application/json",
                         "{\"success\": true, \"message\": \"Last staged file removed\"}");
    }
    // Endpoint: /api/staging/clear (Clear staging)
    else if (basePath == "/api/staging/clear" && method == "POST") {
        string user = extractJsonField(reqBody, "user");
        if (user.empty()) user = "sahil";

        userStagingMap[user].clear();
        sendHttpResponse(clientSock, 200, "OK", "application/json",
                         "{\"success\": true, \"message\": \"Staging area cleared\"}");
    }
    // Endpoint: /api/commit (Commit staged files)
    else if (basePath == "/api/commit" && method == "POST") {
        string user    = extractJsonField(reqBody, "user");
        string message = extractJsonField(reqBody, "message");
        string branch  = extractJsonField(reqBody, "branch");

        if (user.empty()) user = "sahil";
        if (branch.empty()) branch = getUserCurrentBranch(user);
        if (message.empty()) message = "Commit from Web GUI";

        syncWorkingWithBranch(user, branch);
        syncCommitsFromDisk();

        if (userStagingMap[user].isEmpty()) {
            sendHttpResponse(clientSock, 400, "Bad Request", "application/json",
                             "{\"success\": false, \"error\": \"Staging area is empty. Stage files first!\"}");
        } else {
            // Commit staged files
            globalCommitHead = userStagingMap[user].commitAllWithMessage(globalCommitHead, user, branch, message);

            CommitNode* curr = globalCommitHead;
            while (curr != nullptr && curr->next != nullptr) curr = curr->next;
            string newId = (curr != nullptr) ? curr->commitId : "";

            ostringstream oss;
            oss << "{\"success\": true, \"commitId\": \"" << jsonEscape(newId)
                << "\", \"commits\": " << commitsToJson(globalCommitHead) << "}";
            sendHttpResponse(clientSock, 200, "OK", "application/json", oss.str());
        }
    }
    // Endpoint: /api/commits (List all commits)
    else if (basePath == "/api/commits" && method == "GET") {
        syncCommitsFromDisk();
        sendHttpResponse(clientSock, 200, "OK", "application/json", commitsToJson(globalCommitHead));
    }
    // Endpoint: /api/commit/diff (Inspect commit changes compared to parent)
    else if (basePath == "/api/commit/diff" && method == "GET") {
        string commitId = getQueryParam(path, "commitId");

        syncCommitsFromDisk();
        CommitNode* target = findCommit(globalCommitHead, commitId);

        if (target == nullptr) {
            sendHttpResponse(clientSock, 404, "Not Found", "application/json",
                             "{\"error\": \"Commit not found\"}");
        } else {
            string currentDir = "data/objects/" + commitId;
            DIR* dir = opendir(currentDir.c_str());

            ostringstream oss;
            oss << "{\n";
            oss << "  \"commitId\": \"" << jsonEscape(target->commitId) << "\",\n";
            oss << "  \"author\": \"" << jsonEscape(target->author) << "\",\n";
            oss << "  \"message\": \"" << jsonEscape(target->message) << "\",\n";
            oss << "  \"timestamp\": \"" << jsonEscape(target->timestamp) << "\",\n";
            oss << "  \"branch\": \"" << jsonEscape(target->branch) << "\",\n";
            oss << "  \"parentId\": \"" << jsonEscape(target->parentId) << "\",\n";
            oss << "  \"files\": [\n";

            bool firstFile = true;
            if (dir != nullptr) {
                struct dirent* entry;
                while ((entry = readdir(dir)) != nullptr) {
                    string fname = entry->d_name;
                    if (fname == "." || fname == "..") continue;

                    string curPath = currentDir + "/" + fname;
                    string curContent = "", line;
                    ifstream f(curPath.c_str());
                    if (f.is_open()) {
                        while (getline(f, line)) curContent += line + "\n";
                        f.close();
                    }

                    string status = "ADDED";
                    string parentContent = "";

                    if (target->parentId == "NONE") {
                        status = "INITIAL";
                    } else {
                        string parPath = "data/objects/" + target->parentId + "/" + fname;
                        ifstream pf(parPath.c_str());
                        if (pf.is_open()) {
                            while (getline(pf, line)) parentContent += line + "\n";
                            pf.close();
                            status = (parentContent == curContent) ? "UNCHANGED" : "MODIFIED";
                        }
                    }

                    if (!firstFile) oss << ",\n";
                    firstFile = false;
                    oss << "    {\n";
                    oss << "      \"filename\": \"" << jsonEscape(fname) << "\",\n";
                    oss << "      \"status\": \"" << status << "\",\n";
                    oss << "      \"currentContent\": \"" << jsonEscape(curContent) << "\",\n";
                    oss << "      \"parentContent\": \"" << jsonEscape(parentContent) << "\"\n";
                    oss << "    }";
                }
                closedir(dir);
            }
            oss << "\n  ]\n}";
            sendHttpResponse(clientSock, 200, "OK", "application/json", oss.str());
        }
    }
    // Endpoint: /api/checkout (Checkout snapshot to user's working dir)
    else if (basePath == "/api/checkout" && method == "POST") {
        string commitId = extractJsonField(reqBody, "commitId");
        string user     = extractJsonField(reqBody, "user");
        if (user.empty()) user = "sahil";

        syncCommitsFromDisk();
        bool ok = checkoutCommitDirect(globalCommitHead, commitId, user);
        if (ok) {
            sendHttpResponse(clientSock, 200, "OK", "application/json",
                             "{\"success\": true, \"message\": \"Checked out to " + jsonEscape(commitId) + "\"}");
        } else {
            sendHttpResponse(clientSock, 400, "Bad Request", "application/json",
                             "{\"success\": false, \"error\": \"Checkout failed\"}");
        }
    }
    // Endpoint: /api/undo (Undo last commit)
    else if (basePath == "/api/undo" && method == "POST") {
        string user = extractJsonField(reqBody, "user");
        if (user.empty()) user = "sahil";

        syncCommitsFromDisk();
        globalCommitHead = undoLastCommitDirect(globalCommitHead, user);
        sendHttpResponse(clientSock, 200, "OK", "application/json",
                         "{\"success\": true, \"message\": \"Last commit undone\"}");
    }
    // Endpoint: /api/revert (Revert to commit)
    else if (basePath == "/api/revert" && method == "POST") {
        string targetId = extractJsonField(reqBody, "targetId");
        string user     = extractJsonField(reqBody, "user");
        if (user.empty()) user = "sahil";

        syncCommitsFromDisk();
        globalCommitHead = revertToCommitDirect(globalCommitHead, targetId, user);
        sendHttpResponse(clientSock, 200, "OK", "application/json",
                         "{\"success\": true, \"message\": \"Reverted to " + jsonEscape(targetId) + "\"}");
    }
    // Endpoint: /api/contributions (Real-time Contribution stats calculated in C++)
    else if (basePath == "/api/contributions" && method == "GET") {
        string userFilter = getQueryParam(path, "user");
        syncCommitsFromDisk();
        ContributionSummary summary = calculateContributions(globalCommitHead, userFilter);
        string jsonOutput = contributionsToJson(summary);
        sendHttpResponse(clientSock, 200, "OK", "application/json", jsonOutput);
    }
    // Endpoint: /api/contributions/svg (Standalone C++ generated SVG Chart)
    else if (basePath == "/api/contributions/svg" && method == "GET") {
        string userFilter = getQueryParam(path, "user");
        syncCommitsFromDisk();
        ContributionSummary summary = calculateContributions(globalCommitHead, userFilter);
        string svg = generateContributionSvg(summary);
        sendHttpResponse(clientSock, 200, "OK", "image/svg+xml", svg);
    }
    // Endpoint: /api/contributors/svg (Standalone C++ generated Contributor Chart)
    else if (basePath == "/api/contributors/svg" && method == "GET") {
        string userFilter = getQueryParam(path, "user");
        syncCommitsFromDisk();
        ContributionSummary summary = calculateContributions(globalCommitHead, userFilter);
        string svg = generateContributorSvg(summary);
        sendHttpResponse(clientSock, 200, "OK", "image/svg+xml", svg);
    }
    // Endpoint: /api/branches (List all branches with file info)
    else if (basePath == "/api/branches" && method == "GET") {
        string user = getQueryParam(path, "user");
        if (user.empty()) user = "sahil";

        syncCommitsFromDisk();
        string activeBranch = getUserCurrentBranch(user);
        syncWorkingWithBranch(user, activeBranch);
        vector<BranchDetail> branches = getAllBranchesDetailed(globalCommitHead);

        ostringstream oss;
        oss << "[\n";
        for (size_t i = 0; i < branches.size(); i++) {
            const auto& b = branches[i];
            oss << "  {\n";
            oss << "    \"name\": \"" << jsonEscape(b.name) << "\",\n";
            oss << "    \"parentBranch\": \"" << jsonEscape(b.parentBranch) << "\",\n";
            oss << "    \"isCurrent\": " << (b.name == activeBranch ? "true" : "false") << ",\n";
            oss << "    \"latestCommitId\": \"" << jsonEscape(b.latestCommitId) << "\",\n";
            oss << "    \"latestCommitMsg\": \"" << jsonEscape(b.latestCommitMsg) << "\",\n";
            oss << "    \"lastUpdated\": \"" << jsonEscape(b.lastUpdated) << "\",\n";
            oss << "    \"fileCount\": " << b.fileCount << ",\n";
            oss << "    \"files\": [\n";
            for (size_t j = 0; j < b.files.size(); j++) {
                oss << "      \"" << jsonEscape(b.files[j]) << "\"" << (j + 1 < b.files.size() ? "," : "") << "\n";
            }
            oss << "    ]\n";
            oss << "  }" << (i + 1 < branches.size() ? "," : "") << "\n";
        }
        oss << "]";
        sendHttpResponse(clientSock, 200, "OK", "application/json", oss.str());
    }
    // Endpoint: /api/branch (POST create new branch)
    else if (basePath == "/api/branch" && method == "POST") {
        string name = extractJsonField(reqBody, "name");
        string from = extractJsonField(reqBody, "from");
        string user = extractJsonField(reqBody, "user");

        if (user.empty()) user = "sahil";
        if (from.empty()) from = getUserCurrentBranch(user);

        if (name.empty()) {
            sendHttpResponse(clientSock, 400, "Bad Request", "application/json",
                             "{\"success\": false, \"error\": \"Branch name cannot be empty\"}");
        } else {
            bool ok = createBranch(name, from, user);
            if (ok) {
                sendHttpResponse(clientSock, 200, "OK", "application/json",
                                 "{\"success\": true, \"name\": \"" + jsonEscape(name) +
                                 "\", \"message\": \"Branch '" + jsonEscape(name) + "' created successfully!\"}");
            } else {
                sendHttpResponse(clientSock, 400, "Bad Request", "application/json",
                                 "{\"success\": false, \"error\": \"Could not create branch. Invalid name or directory error.\"}");
            }
        }
    }
    // Endpoint: /api/branch/switch (POST checkout/switch branch)
    else if (basePath == "/api/branch/switch" && method == "POST") {
        string branch = extractJsonField(reqBody, "branch");
        string user   = extractJsonField(reqBody, "user");

        if (user.empty()) user = "sahil";
        if (branch.empty()) branch = "main";

        bool ok = switchBranch(user, branch);
        if (ok) {
            sendHttpResponse(clientSock, 200, "OK", "application/json",
                             "{\"success\": true, \"currentBranch\": \"" + jsonEscape(branch) +
                             "\", \"message\": \"Switched to branch " + jsonEscape(branch) + "\"}");
        } else {
            sendHttpResponse(clientSock, 400, "Bad Request", "application/json",
                             "{\"success\": false, \"error\": \"Failed to switch branch.\"}");
        }
    }
    // Endpoint: /api/branch/files (GET files inside a branch folder)
    else if (basePath == "/api/branch/files" && method == "GET") {
        string branch = getQueryParam(path, "branch");
        if (branch.empty()) branch = "main";

        vector<string> files = getBranchFiles(branch);
        string branchDir = "data/branches/" + branch;

        ostringstream oss;
        oss << "[\n";
        for (size_t i = 0; i < files.size(); i++) {
            string filePath = branchDir + "/" + files[i];
            struct stat st;
            long size = 0;
            if (stat(filePath.c_str(), &st) == 0) {
                size = st.st_size;
            }
            oss << "  {\"name\": \"" << jsonEscape(files[i]) << "\", \"size\": " << size << "}";
            if (i + 1 < files.size()) oss << ",";
            oss << "\n";
        }
        oss << "]";
        sendHttpResponse(clientSock, 200, "OK", "application/json", oss.str());
    }
    // Endpoint: /api/branch/file (GET content of a file in a branch)
    else if (basePath == "/api/branch/file" && method == "GET") {
        string branch = getQueryParam(path, "branch");
        string fname  = getQueryParam(path, "name");
        if (branch.empty()) branch = "main";

        string content = readBranchFileContent(branch, fname);
        ostringstream oss;
        oss << "{\"name\": \"" << jsonEscape(fname)
            << "\", \"branch\": \"" << jsonEscape(branch)
            << "\", \"content\": \"" << jsonEscape(content) << "\"}";
        sendHttpResponse(clientSock, 200, "OK", "application/json", oss.str());
    }
    // Endpoint: /api/branch (DELETE remove a branch)
    else if (basePath == "/api/branch" && method == "DELETE") {
        string branch = getQueryParam(path, "name");
        string errorMsg = "";
        bool ok = removeBranch(branch, errorMsg);
        if (ok) {
            sendHttpResponse(clientSock, 200, "OK", "application/json",
                             "{\"success\": true, \"message\": \"Branch deleted successfully\"}");
        } else {
            sendHttpResponse(clientSock, 400, "Bad Request", "application/json",
                             "{\"success\": false, \"error\": \"" + jsonEscape(errorMsg) + "\"}");
        }
    }
    // Serve Static React Frontend assets
    else if (method == "GET" && basePath.rfind("/api", 0) != 0) {
        string filePath = "frontend/dist" + basePath;
        if (basePath == "/" || basePath.empty()) {
            filePath = "frontend/dist/index.html";
        }

        string mimeType = "text/plain";
        if (filePath.find(".html") != string::npos) mimeType = "text/html; charset=utf-8";
        else if (filePath.find(".js") != string::npos) mimeType = "application/javascript; charset=utf-8";
        else if (filePath.find(".css") != string::npos) mimeType = "text/css; charset=utf-8";
        else if (filePath.find(".svg") != string::npos) mimeType = "image/svg+xml";
        else if (filePath.find(".png") != string::npos) mimeType = "image/png";
        else if (filePath.find(".ico") != string::npos) mimeType = "image/x-icon";
        else if (filePath.find(".json") != string::npos) mimeType = "application/json";

        ifstream f(filePath.c_str(), ios::binary);
        if (f.is_open()) {
            ostringstream ss;
            ss << f.rdbuf();
            string content = ss.str();
            f.close();
            sendHttpResponse(clientSock, 200, "OK", mimeType, content);
        } else {
            ifstream fallback("frontend/dist/index.html", ios::binary);
            if (fallback.is_open()) {
                ostringstream ss;
                ss << fallback.rdbuf();
                string content = ss.str();
                fallback.close();
                sendHttpResponse(clientSock, 200, "OK", "text/html; charset=utf-8", content);
            } else {
                sendHttpResponse(clientSock, 404, "Not Found", "application/json",
                                 "{\"error\": \"Route not found. Build frontend with: cd frontend && npm run build\"}");
            }
        }
    }
    else {
        sendHttpResponse(clientSock, 404, "Not Found", "application/json",
                         "{\"error\": \"Route not found: " + jsonEscape(basePath) + "\"}");
    }

    close(clientSock);
}

int main(int argc, char* argv[]) {
    int port = PORT;
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    // Load initial commit history and branch system
    globalCommitHead = loadCommits();
    initBranchSystem(globalCommitHead);
    cout << "========================================\n";
    cout << "  MiniVCS C++ REST API Server Started   \n";
    cout << "========================================\n";

    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0) {
        cerr << "Failed to create socket.\n";
        return 1;
    }

    int opt = 1;
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (::bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Failed to bind to port " << port << ".\n";
        close(serverSock);
        return 1;
    }

    if (::listen(serverSock, 20) < 0) {
        cerr << "Listen failed.\n";
        close(serverSock);
        return 1;
    }

    cout << "Backend listening on http://localhost:" << port << "\n";
    cout << "Serving REST API endpoints with live C++ contribution engine...\n";

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSock = ::accept(serverSock, (struct sockaddr*)&clientAddr, &clientLen);

        if (clientSock >= 0) {
            thread t(handleClient, clientSock);
            t.detach();
        }
    }

    if (globalCommitHead != nullptr) {
        freeCommits(globalCommitHead);
    }

    close(serverSock);
    return 0;
}
