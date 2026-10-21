// API client for MiniVCS C++ Server

const API_BASE = ''; // Uses Vite proxy to http://localhost:8080

export const api = {
  // Status
  async getStatus() {
    const res = await fetch(`${API_BASE}/api/status`);
    return res.json();
  },

  // Auth & Users
  async getUsers() {
    const res = await fetch(`${API_BASE}/api/users`);
    return res.json();
  },

  async login(username, password) {
    const res = await fetch(`${API_BASE}/api/login`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ username, password })
    });
    return res.json();
  },

  async register(username, password) {
    const res = await fetch(`${API_BASE}/api/register`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ username, password })
    });
    return res.json();
  },

  // Working Directory Files
  async getFiles(user) {
    const res = await fetch(`${API_BASE}/api/files?user=${encodeURIComponent(user)}&_t=${Date.now()}`, {
      cache: 'no-store'
    });
    return res.json();
  },

  async getFileContent(user, filename) {
    const res = await fetch(`${API_BASE}/api/file?user=${encodeURIComponent(user)}&name=${encodeURIComponent(filename)}&_t=${Date.now()}`, {
      cache: 'no-store'
    });
    return res.json();
  },

  async saveFile(user, filename, content) {
    const res = await fetch(`${API_BASE}/api/file`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ user, name: filename, content })
    });
    return res.json();
  },

  async deleteFile(user, filename) {
    const res = await fetch(`${API_BASE}/api/file?user=${encodeURIComponent(user)}&name=${encodeURIComponent(filename)}`, {
      method: 'DELETE'
    });
    return res.json();
  },

  // Staging Area
  async getStaging(user) {
    const res = await fetch(`${API_BASE}/api/staging?user=${encodeURIComponent(user)}`);
    return res.json();
  },

  async stageFile(user, filename) {
    const res = await fetch(`${API_BASE}/api/staging/add`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ user, filename })
    });
    return res.json();
  },

  async unstageFile(user) {
    const res = await fetch(`${API_BASE}/api/staging/unstage`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ user })
    });
    return res.json();
  },

  async clearStaging(user) {
    const res = await fetch(`${API_BASE}/api/staging/clear`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ user })
    });
    return res.json();
  },

  // Commits
  async getCommits() {
    const res = await fetch(`${API_BASE}/api/commits`);
    return res.json();
  },

  async createCommit(user, message, branch = 'main') {
    const res = await fetch(`${API_BASE}/api/commit`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ user, message, branch })
    });
    return res.json();
  },

  async getCommitDiff(commitId) {
    const res = await fetch(`${API_BASE}/api/commit/diff?commitId=${encodeURIComponent(commitId)}`);
    return res.json();
  },

  async checkoutCommit(commitId, user) {
    const res = await fetch(`${API_BASE}/api/checkout`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ commitId, user })
    });
    return res.json();
  },

  async undoLastCommit(user) {
    const res = await fetch(`${API_BASE}/api/undo`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ user })
    });
    return res.json();
  },

  async revertToCommit(targetId, user) {
    const res = await fetch(`${API_BASE}/api/revert`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ targetId, user })
    });
    return res.json();
  },

  // Contributions (Real-Time C++ calculation)
  async getContributions(user = '') {
    const res = await fetch(`${API_BASE}/api/contributions?user=${encodeURIComponent(user)}`);
    return res.json();
  },

  getContributionSvgUrl(user = '') {
    return `${API_BASE}/api/contributions/svg?user=${encodeURIComponent(user)}&t=${Date.now()}`;
  },

  // Branches
  async getBranches(user = 'sahil') {
    const res = await fetch(`${API_BASE}/api/branches?user=${encodeURIComponent(user)}&_t=${Date.now()}`, {
      cache: 'no-store'
    });
    return res.json();
  },

  async createBranch(name, from = 'main', user = 'sahil') {
    const res = await fetch(`${API_BASE}/api/branch`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ name, from, user })
    });
    return res.json();
  },

  async switchBranch(branch, user = 'sahil') {
    const res = await fetch(`${API_BASE}/api/branch/switch`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ branch, user })
    });
    return res.json();
  },

  async getBranchFiles(branch) {
    const res = await fetch(`${API_BASE}/api/branch/files?branch=${encodeURIComponent(branch)}&_t=${Date.now()}`, {
      cache: 'no-store'
    });
    return res.json();
  },

  async getBranchFileContent(branch, filename) {
    const res = await fetch(`${API_BASE}/api/branch/file?branch=${encodeURIComponent(branch)}&name=${encodeURIComponent(filename)}&_t=${Date.now()}`, {
      cache: 'no-store'
    });
    return res.json();
  },

  async deleteBranch(name) {
    const res = await fetch(`${API_BASE}/api/branch?name=${encodeURIComponent(name)}`, {
      method: 'DELETE'
    });
    return res.json();
  }
};

