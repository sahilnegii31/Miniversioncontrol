import React, { useState } from 'react';
import { GitBranch, User, Server, Code, BarChart2 } from 'lucide-react';

export default function Navbar({
  currentUser,
  users,
  onSwitchUser,
  onLogin,
  onRegister,
  backendConnected,
  currentTab = 'code',
  onTabChange
}) {
  const [showAuthModal, setShowAuthModal] = useState(false);
  const [isRegister, setIsRegister] = useState(false);
  const [usernameInput, setUsernameInput] = useState('');
  const [passwordInput, setPasswordInput] = useState('');
  const [authError, setAuthError] = useState('');

  const handleAuthSubmit = async (e) => {
    e.preventDefault();
    setAuthError('');
    if (!usernameInput.trim() || !passwordInput.trim()) {
      setAuthError('Please fill in both username and password');
      return;
    }

    try {
      if (isRegister) {
        await onRegister(usernameInput.trim(), passwordInput.trim());
      } else {
        await onLogin(usernameInput.trim(), passwordInput.trim());
      }
      setShowAuthModal(false);
      setUsernameInput('');
      setPasswordInput('');
    } catch (err) {
      setAuthError(err.message || 'Authentication failed');
    }
  };

  return (
    <header className="navbar">
      <div className="nav-left">
        <div className="logo-box">
          <GitBranch size={22} className="text-emerald-400" />
          <span className="logo-title">MiniVCS</span>
          <span className="version-pill">C++ Powered</span>
        </div>

        <div className="connection-badge">
          <span className={`status-dot ${backendConnected ? 'connected' : 'disconnected'}`} />
          <Server size={14} className="text-gray-400" />
          <span className="text-xs text-gray-300">
            {backendConnected ? 'C++ REST Backend Active' : 'Connecting to C++...'}
          </span>
        </div>
      </div>

      {/* GitHub-style Main Navigation Tabs */}
      <nav className="nav-tabs">
        <button
          onClick={() => onTabChange && onTabChange('code')}
          className={`nav-tab-btn ${currentTab === 'code' ? 'active' : ''}`}
        >
          <Code size={16} /> Code
        </button>
        <button
          onClick={() => onTabChange && onTabChange('insights')}
          className={`nav-tab-btn ${currentTab === 'insights' ? 'active' : ''}`}
        >
          <BarChart2 size={16} /> Insights
        </button>
      </nav>

      <div className="nav-right">
        {/* Quick User Switcher */}
        <div className="user-dropdown">
          <span className="text-xs text-gray-400">User:</span>
          <select
            value={currentUser}
            onChange={(e) => onSwitchUser(e.target.value)}
            className="user-select"
          >
            {users.map((u) => (
              <option key={u.username} value={u.username}>
                {u.username}
              </option>
            ))}
          </select>
        </div>

        {/* Login / Register Button */}
        <button
          onClick={() => {
            setIsRegister(false);
            setAuthError('');
            setShowAuthModal(true);
          }}
          className="btn-sm btn-outline flex-center gap-1.5"
        >
          <User size={14} /> Account
        </button>
      </div>

      {/* Auth Modal */}
      {showAuthModal && (
        <div className="modal-backdrop">
          <div className="modal-box">
            <h4 className="modal-title">
              {isRegister ? 'Register MiniVCS Account' : 'Login to MiniVCS'}
            </h4>
            <p className="modal-subtitle">Directly stored in C++ data/users.txt</p>

            {authError && <div className="commit-error">{authError}</div>}

            <form onSubmit={handleAuthSubmit}>
              <div className="form-group mb-3">
                <label className="form-label">Username</label>
                <input
                  type="text"
                  value={usernameInput}
                  onChange={(e) => setUsernameInput(e.target.value)}
                  className="form-input"
                  placeholder="e.g. sahil"
                  autoFocus
                />
              </div>

              <div className="form-group mb-4">
                <label className="form-label">Password</label>
                <input
                  type="password"
                  value={passwordInput}
                  onChange={(e) => setPasswordInput(e.target.value)}
                  className="form-input"
                  placeholder="••••••••"
                />
              </div>

              <div className="modal-btn-row">
                <button
                  type="button"
                  onClick={() => setIsRegister(!isRegister)}
                  className="text-xs text-emerald-400 hover:underline mr-auto"
                >
                  {isRegister ? 'Already have an account? Login' : "Don't have an account? Register"}
                </button>

                <button
                  type="button"
                  onClick={() => setShowAuthModal(false)}
                  className="btn-sm btn-outline"
                >
                  Cancel
                </button>
                <button type="submit" className="btn-sm btn-primary">
                  {isRegister ? 'Register' : 'Login'}
                </button>
              </div>
            </form>
          </div>
        </div>
      )}
    </header>
  );
}

