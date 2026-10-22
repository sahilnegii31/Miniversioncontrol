import React, { useState, useEffect, useCallback } from 'react';
import { api } from './api';
import Navbar from './components/Navbar';
import InsightsPage from './components/InsightsPage';
import WorkingDirExplorer from './components/WorkingDirExplorer';
import StagingPanel from './components/StagingPanel';
import CommitHistory from './components/CommitHistory';
import BranchesPage from './components/BranchesPage';
import './App.css';

export default function App() {
  const [currentTab, setCurrentTab] = useState('code'); // 'code', 'branches', or 'insights'
  const [currentUser, setCurrentUser] = useState('sahil');
  const [users, setUsers] = useState([{ username: 'sahil' }, { username: 'Mohit' }]);
  const [currentBranch, setCurrentBranch] = useState('main');
  const [branches, setBranches] = useState([]);
  const [files, setFiles] = useState([]);
  const [stagedFiles, setStagedFiles] = useState([]);
  const [commits, setCommits] = useState([]);
  const [contributions, setContributions] = useState(null);
  const [selectedAuthor, setSelectedAuthor] = useState('');
  const [backendConnected, setBackendConnected] = useState(false);
  const [toast, setToast] = useState('');

  const showToast = (msg) => {
    setToast(msg);
    setTimeout(() => setToast(''), 4000);
  };

  // Load branches
  const loadBranches = useCallback(async (user) => {
    try {
      const data = await api.getBranches(user);
      setBranches(data || []);
      const active = (data || []).find((b) => b.isActive);
      if (active) {
        setCurrentBranch(active.name);
      }
    } catch (err) {
      console.error('Failed to load branches:', err);
    }
  }, []);

  // Load contributions in real-time
  const loadContributions = useCallback(async (authorFilter = '') => {
    try {
      const data = await api.getContributions(authorFilter);
      setContributions(data);
    } catch (err) {
      console.error('Failed to load contributions:', err);
    }
  }, []);

  // Load working directory files
  const loadFiles = useCallback(async (user) => {
    try {
      const data = await api.getFiles(user);
      setFiles(data);
    } catch (err) {
      console.error('Failed to load files:', err);
    }
  }, []);

  // Load staged files
  const loadStaging = useCallback(async (user) => {
    try {
      const data = await api.getStaging(user);
      setStagedFiles(data);
    } catch (err) {
      console.error('Failed to load staging:', err);
    }
  }, []);

  // Load commit history
  const loadCommits = useCallback(async () => {
    try {
      const data = await api.getCommits();
      setCommits(data);
    } catch (err) {
      console.error('Failed to load commits:', err);
    }
  }, []);

  // Check backend health & initial load
  const loadAll = useCallback(async () => {
    try {
      await api.getStatus();
      setBackendConnected(true);

      const userList = await api.getUsers();
      if (userList && userList.length > 0) {
        setUsers(userList);
      }

      await Promise.all([
        loadFiles(currentUser),
        loadStaging(currentUser),
        loadCommits(),
        loadBranches(currentUser),
        loadContributions(selectedAuthor)
      ]);
    } catch {
      setBackendConnected(false);
    }
  }, [currentUser, selectedAuthor, loadFiles, loadStaging, loadCommits, loadBranches, loadContributions]);

  useEffect(() => {
    loadAll();
    const interval = setInterval(() => {
      // Periodic background sync
      loadContributions(selectedAuthor);
    }, 10000);
    return () => clearInterval(interval);
  }, [loadAll, selectedAuthor, loadContributions]);

  // Handle user switch
  const handleSwitchUser = (newUname) => {
    setCurrentUser(newUname);
    loadFiles(newUname);
    loadStaging(newUname);
    loadBranches(newUname);
    showToast(`Switched active user to ${newUname}`);
  };

  // File Operations
  const handleSaveFile = async (filename, content) => {
    try {
      const res = await api.saveFile(currentUser, filename, content);
      if (!res.success) throw new Error(res.error || 'Failed to save');
      await loadFiles(currentUser);
      showToast(`Saved ${filename}`);
    } catch (err) {
      showToast(`Error: ${err.message}`);
      throw err;
    }
  };

  const handleDeleteFile = async (filename) => {
    await api.deleteFile(currentUser, filename);
    await loadFiles(currentUser);
    showToast(`Deleted ${filename}`);
  };

  // Staging Operations
  const handleStageFile = async (filename) => {
    const res = await api.stageFile(currentUser, filename);
    if (res.success) {
      await loadStaging(currentUser);
      showToast(`Staged ${filename} (git add)`);
    } else {
      showToast(res.error || 'Failed to stage file');
    }
  };

  const handleUnstage = async () => {
    await api.unstageFile(currentUser);
    await loadStaging(currentUser);
    showToast('Unstaged last file');
  };

  const handleClearStaging = async () => {
    await api.clearStaging(currentUser);
    await loadStaging(currentUser);
    showToast('Cleared staging area');
  };

  // Real-Time Commit Operation
  const handleCommit = async (message, branch) => {
    const res = await api.createCommit(currentUser, message, branch);
    if (res.success) {
      showToast(`🎉 Commit ${res.commitId} created! Insights & branch updated.`);
      // Update staging, commits, branches and contribution metrics in real time!
      await Promise.all([
        loadStaging(currentUser),
        loadCommits(),
        loadBranches(currentUser),
        loadContributions(selectedAuthor)
      ]);
    } else {
      throw new Error(res.error || 'Failed to commit');
    }
  };

  // Branch Operations
  const handleSwitchBranch = async (targetBranch) => {
    try {
      const res = await api.switchBranch(targetBranch, currentUser);
      if (!res.success) {
        throw new Error(res.error || 'Failed to switch branch');
      }
      setCurrentBranch(targetBranch);
      showToast(`Switched active branch to "${targetBranch}"`);
      await Promise.all([
        loadFiles(currentUser),
        loadStaging(currentUser),
        loadBranches(currentUser),
        loadContributions(selectedAuthor)
      ]);
    } catch (err) {
      showToast(`Error switching branch: ${err.message}`);
    }
  };

  const handleCreateBranch = async (branchName, fromBranch, switchNow = true) => {
    try {
      const res = await api.createBranch(branchName, fromBranch, currentUser);
      if (!res.success) {
        throw new Error(res.error || 'Failed to create branch');
      }
      showToast(`🎉 Branch "${branchName}" created from "${fromBranch}"!`);
      if (switchNow) {
        await handleSwitchBranch(branchName);
      } else {
        await loadBranches(currentUser);
      }
    } catch (err) {
      showToast(`Error creating branch: ${err.message}`);
      throw err;
    }
  };

  const handleDeleteBranch = async (branchName) => {
    try {
      const res = await api.deleteBranch(branchName);
      if (!res.success) {
        throw new Error(res.error || 'Failed to delete branch');
      }
      showToast(`Deleted branch "${branchName}"`);
      await loadBranches(currentUser);
    } catch (err) {
      showToast(`Error deleting branch: ${err.message}`);
      throw err;
    }
  };

  // Checkout, Undo, Revert
  const handleCheckout = async (commitId) => {
    const res = await api.checkoutCommit(commitId, currentUser);
    if (res.success) {
      await loadFiles(currentUser);
      showToast(`Checked out commit ${commitId}`);
    } else {
      showToast(res.error || 'Checkout failed');
    }
  };

  const handleUndoLast = async () => {
    const res = await api.undoLastCommit(currentUser);
    if (res.success) {
      await Promise.all([
        loadCommits(),
        loadFiles(currentUser),
        loadBranches(currentUser),
        loadContributions(selectedAuthor)
      ]);
      showToast('Undid last commit');
    } else {
      showToast(res.error || 'Undo failed');
    }
  };

  const handleRevertTo = async (targetId) => {
    const res = await api.revertToCommit(targetId, currentUser);
    if (res.success) {
      await Promise.all([
        loadCommits(),
        loadFiles(currentUser),
        loadBranches(currentUser),
        loadContributions(selectedAuthor)
      ]);
      showToast(`Reverted back to commit ${targetId}`);
    } else {
      showToast(res.error || 'Revert failed');
    }
  };

  return (
    <div className="app-layout">
      {/* Top Navigation Bar with Code, Branches & Insights Tabs */}
      <Navbar
        currentUser={currentUser}
        users={users}
        onSwitchUser={handleSwitchUser}
        currentTab={currentTab}
        onTabChange={setCurrentTab}
        branches={branches}
        currentBranch={currentBranch}
        onSwitchBranch={handleSwitchBranch}
        onLogin={async (u, p) => {
          const res = await api.login(u, p);
          if (res.success) {
            handleSwitchUser(u);
            showToast(`Logged in as ${u}`);
          } else {
            throw new Error(res.error);
          }
        }}
        onRegister={async (u, p) => {
          const res = await api.register(u, p);
          if (res.success) {
            const list = await api.getUsers();
            setUsers(list);
            handleSwitchUser(u);
            showToast(`Registered and logged in as ${u}`);
          } else {
            throw new Error(res.error);
          }
        }}
        backendConnected={backendConnected}
      />

      {/* Main Container */}
      <main className="main-content">
        {/* ================= VIEW 1: CODE TAB ================= */}
        {currentTab === 'code' && (
          <div className="tab-fade-in">
            {/* 2-Column Split: Working Directory on Left, Staging & Commit on Right */}
            <div className="split-grid mb-6">
              <WorkingDirExplorer
                user={currentUser}
                currentBranch={currentBranch}
                files={files}
                onRefresh={() => loadFiles(currentUser)}
                onStageFile={handleStageFile}
                onSaveFile={handleSaveFile}
                onDeleteFile={handleDeleteFile}
              />

              <StagingPanel
                stagedFiles={stagedFiles}
                onUnstage={handleUnstage}
                onClearStaging={handleClearStaging}
                onCommit={handleCommit}
                currentBranch={currentBranch}
              />
            </div>

            {/* Commit History Timeline & Diff Inspection */}
            <section className="mb-8">
              <CommitHistory
                commits={commits}
                onCheckout={handleCheckout}
                onUndoLast={handleUndoLast}
                onRevertTo={handleRevertTo}
              />
            </section>
          </div>
        )}

        {/* ================= VIEW 2: BRANCHES TAB ================= */}
        {currentTab === 'branches' && (
          <div className="tab-fade-in">
            <BranchesPage
              branches={branches}
              currentBranch={currentBranch}
              currentUser={currentUser}
              onSwitchBranch={handleSwitchBranch}
              onCreateBranch={handleCreateBranch}
              onDeleteBranch={handleDeleteBranch}
              onRefresh={() => loadBranches(currentUser)}
            />
          </div>
        )}

        {/* ================= VIEW 3: INSIGHTS TAB (Pulse, Commits, Contributors) ================= */}
        {currentTab === 'insights' && (
          <div className="tab-fade-in">
            <InsightsPage
              contributions={contributions}
              commits={commits}
              files={files}
              selectedAuthor={selectedAuthor}
              onSelectAuthor={(auth) => {
                setSelectedAuthor(auth);
                loadContributions(auth);
              }}
              isLive={backendConnected}
            />
          </div>
        )}
      </main>

      {/* Floating Notification Toast */}
      {toast && (
        <div className="toast-notification">
          <span>{toast}</span>
        </div>
      )}
    </div>
  );
}
