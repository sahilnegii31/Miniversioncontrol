import React, { useState } from 'react';
import { GitBranch, Plus, Check, Trash2, Eye, Folder, FileText, ArrowRightCircle, RefreshCw, X, Search, GitFork, CornerDownRight } from 'lucide-react';
import { api } from '../api';

export default function BranchesPage({
  branches,
  currentBranch,
  currentUser,
  onSwitchBranch,
  onCreateBranch,
  onDeleteBranch,
  onRefresh
}) {
  const [showCreateModal, setShowCreateModal] = useState(false);
  const [newBranchName, setNewBranchName] = useState('');
  const [sourceBranch, setSourceBranch] = useState(currentBranch || 'main');
  const [switchAfterCreate, setSwitchAfterCreate] = useState(true);
  const [searchFilter, setSearchFilter] = useState('');

  // Inspector modal state for viewing branch files
  const [inspectingBranch, setInspectingBranch] = useState(null);
  const [branchFiles, setBranchFiles] = useState([]);
  const [loadingFiles, setLoadingFiles] = useState(false);
  const [selectedFileContent, setSelectedFileContent] = useState(null);
  const [selectedFileName, setSelectedFileName] = useState('');

  const handleOpenInspector = async (branchName) => {
    setInspectingBranch(branchName);
    setSelectedFileContent(null);
    setSelectedFileName('');
    setLoadingFiles(true);
    try {
      const files = await api.getBranchFiles(branchName);
      setBranchFiles(files || []);
      if (files && files.length > 0) {
        handleOpenFileContent(branchName, files[0].name);
      }
    } catch (err) {
      console.error('Failed to load branch files:', err);
    } finally {
      setLoadingFiles(false);
    }
  };

  const handleOpenFileContent = async (branchName, fname) => {
    setSelectedFileName(fname);
    try {
      const data = await api.getBranchFileContent(branchName, fname);
      setSelectedFileContent(data.content || '');
    } catch (err) {
      setSelectedFileContent('Failed to load file content.');
    }
  };

  const handleCreateSubmit = (e) => {
    e.preventDefault();
    const clean = newBranchName.trim();
    if (!clean) return;
    onCreateBranch(clean, sourceBranch, switchAfterCreate);
    setNewBranchName('');
    setShowCreateModal(false);
  };

  const defaultBranch = branches.find((b) => b.name === 'main') || {
    name: 'main',
    isCurrent: currentBranch === 'main',
    latestCommitId: 'NONE',
    latestCommitMsg: 'Default branch',
    fileCount: 0,
    files: []
  };

  const otherBranches = branches.filter((b) => b.name !== 'main');
  const filteredOther = otherBranches.filter((b) =>
    b.name.toLowerCase().includes(searchFilter.toLowerCase())
  );

  return (
    <div className="branches-container">
      {/* 1. Header Card */}
      <div className="branches-header-card">
        <div className="branches-header-left">
          <div className="branches-icon-badge">
            <GitBranch size={26} />
          </div>
          <div>
            <div className="branches-header-title-row">
              <h2 className="branches-header-title">Branch Management</h2>
              <span className="count-pill">{branches.length} total branches</span>
              <span className="branch-badge badge-active">
                <Check size={12} /> Active: {currentBranch}
              </span>
            </div>
            <p className="branches-header-subtitle">
              Physical Directory Snapshots — Each branch maintains its own isolated folder at{' '}
              <code className="branch-meta-path">data/branches/&lt;name&gt;/</code>
            </p>
          </div>
        </div>

        <div className="branches-header-actions">
          <button
            onClick={onRefresh}
            className="btn-sm btn-outline"
            title="Refresh branches list"
          >
            <RefreshCw size={14} /> Refresh
          </button>
          <button
            onClick={() => {
              setSourceBranch(currentBranch || 'main');
              setShowCreateModal(true);
            }}
            className="btn-sm btn-primary"
          >
            <Plus size={15} /> New Branch
          </button>
        </div>
      </div>

      {/* 2. Visual Branch Tree & Search Bar */}
      <div className="panel-card" style={{ padding: '16px 20px' }}>
        <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '12px', flexWrap: 'wrap', gap: '10px' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
            <GitFork size={16} style={{ color: 'var(--accent-purple)' }} />
            <span style={{ fontWeight: 600, fontSize: '0.9rem' }}>N-ary Branch Hierarchy</span>
            <span style={{ fontSize: '0.75rem', color: 'var(--text-muted)' }}>(Tree Traversal)</span>
          </div>

          <div style={{ display: 'flex', alignItems: 'center', gap: '8px', position: 'relative' }}>
            <Search size={14} style={{ color: 'var(--text-muted)', position: 'absolute', left: '10px' }} />
            <input
              type="text"
              placeholder="Filter branches..."
              value={searchFilter}
              onChange={(e) => setSearchFilter(e.target.value)}
              className="branch-form-input"
              style={{ paddingLeft: '32px', paddingRight: '12px', height: '32px', fontSize: '0.8rem', width: '220px' }}
            />
          </div>
        </div>

        <div className="branch-tree-box">
          <div>🌳 Branch Hierarchy Tree (Root: main)</div>
          <div style={{ color: '#3fb950', marginTop: '4px' }}>
            main {defaultBranch.name === currentBranch ? '★ [ACTIVE]' : ''} ({defaultBranch.fileCount} files)
          </div>
          {otherBranches.length === 0 ? (
            <div style={{ color: '#6e7681', paddingLeft: '20px' }}>
              └── (no custom branches created yet)
            </div>
          ) : (
            otherBranches.map((b, idx) => {
              const isLast = idx === otherBranches.length - 1;
              const prefix = isLast ? '└── ' : '├── ';
              return (
                <div key={b.name} style={{ paddingLeft: '20px', color: b.name === currentBranch ? '#58a6ff' : '#d2a8ff' }}>
                  {prefix}{b.name} (from: {b.parentBranch || 'main'}) {b.name === currentBranch ? '★ [ACTIVE]' : ''} [{b.fileCount} files]
                </div>
              );
            })
          )}
        </div>
      </div>

      {/* 3. Default Branch Section */}
      <div className="panel-card" style={{ padding: '20px' }}>
        <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: '14px', borderBottom: '1px solid var(--border-color)', paddingBottom: '10px' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
            <GitBranch size={16} style={{ color: '#3fb950' }} />
            <span style={{ fontWeight: 600, fontSize: '0.95rem' }}>Default Branch</span>
            <span className="branch-badge badge-default">Default</span>
          </div>
          <span className="branch-meta-path">data/branches/main/</span>
        </div>

        <div className={`branch-card default-branch ${defaultBranch.name === currentBranch ? 'active-branch' : ''}`}>
          <div className="branch-card-left">
            <div className="branch-avatar-icon main">
              <GitBranch size={18} />
            </div>
            <div className="branch-info-col">
              <div className="branch-name-row">
                <span className="branch-name-text">main</span>
                {defaultBranch.name === currentBranch && (
                  <span className="branch-badge badge-active">
                    <Check size={12} /> Checked Out (@{currentUser})
                  </span>
                )}
              </div>
              <div className="branch-meta-row">
                <span className="branch-meta-item">
                  Commit: <strong style={{ color: '#e6edf3', fontFamily: 'monospace' }}>{defaultBranch.latestCommitId}</strong>
                </span>
                <span>•</span>
                <span className="branch-meta-item">
                  Files: <strong style={{ color: 'var(--accent-purple)' }}>{defaultBranch.fileCount}</strong>
                </span>
                <span>•</span>
                <span className="branch-meta-item" style={{ maxWidth: '320px', overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap' }}>
                  "{defaultBranch.latestCommitMsg}"
                </span>
                {defaultBranch.lastUpdated && (
                  <>
                    <span>•</span>
                    <span style={{ color: 'var(--text-muted)' }}>{defaultBranch.lastUpdated}</span>
                  </>
                )}
              </div>
            </div>
          </div>

          <div className="branch-card-actions">
            <button
              onClick={() => handleOpenInspector('main')}
              className="btn-sm btn-outline"
              title="Inspect files in data/branches/main/"
            >
              <Eye size={14} /> View Files ({defaultBranch.fileCount})
            </button>
            {defaultBranch.name !== currentBranch && (
              <button
                onClick={() => onSwitchBranch('main')}
                className="btn-sm btn-emerald"
                title="Switch active branch to main"
              >
                <ArrowRightCircle size={14} /> Switch to main
              </button>
            )}
          </div>
        </div>
      </div>

      {/* 4. Other Branches Section */}
      <div className="panel-card" style={{ padding: '20px' }}>
        <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: '14px', borderBottom: '1px solid var(--border-color)', paddingBottom: '10px' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
            <Folder size={16} style={{ color: 'var(--accent-purple)' }} />
            <span style={{ fontWeight: 600, fontSize: '0.95rem' }}>Feature & Development Branches ({otherBranches.length})</span>
          </div>
          <span style={{ fontSize: '0.75rem', color: 'var(--text-muted)' }}>Isolated workspace folders</span>
        </div>

        {otherBranches.length === 0 ? (
          <div style={{ padding: '40px 20px', textAlign: 'center', color: 'var(--text-muted)' }}>
            <GitBranch size={36} style={{ margin: '0 auto 12px auto', opacity: 0.4 }} />
            <p style={{ fontSize: '0.95rem', fontWeight: 500, color: 'var(--text-secondary)' }}>No custom branches created yet.</p>
            <p style={{ fontSize: '0.8rem', marginTop: '6px' }}>
              Click <strong style={{ color: 'var(--text-primary)' }}>"New Branch"</strong> to branch off with your own physical directory and files.
            </p>
          </div>
        ) : filteredOther.length === 0 ? (
          <div style={{ padding: '30px 20px', textAlign: 'center', color: 'var(--text-muted)', fontSize: '0.85rem' }}>
            No branches match "{searchFilter}".
          </div>
        ) : (
          <div>
            {filteredOther.map((b) => {
              const isActive = b.name === currentBranch;
              return (
                <div
                  key={b.name}
                  className={`branch-card ${isActive ? 'active-branch' : ''}`}
                >
                  <div className="branch-card-left">
                    <div className="branch-avatar-icon feature">
                      <GitBranch size={18} />
                    </div>
                    <div className="branch-info-col">
                      <div className="branch-name-row">
                        <span className="branch-name-text">{b.name}</span>
                        {isActive && (
                          <span className="branch-badge badge-active">
                            <Check size={12} /> Checked Out (@{currentUser})
                          </span>
                        )}
                        <span style={{ fontSize: '0.75rem', color: 'var(--text-muted)', display: 'inline-flex', alignItems: 'center', gap: '4px' }}>
                          <CornerDownRight size={12} /> from <strong style={{ color: '#3fb950' }}>{b.parentBranch || 'main'}</strong>
                        </span>
                      </div>
                      <div className="branch-meta-row">
                        <span className="branch-meta-path">data/branches/{b.name}/</span>
                        <span>•</span>
                        <span className="branch-meta-item">
                          Files: <strong style={{ color: 'var(--accent-purple)' }}>{b.fileCount}</strong>
                        </span>
                        <span>•</span>
                        <span className="branch-meta-item">
                          Commit: <strong style={{ color: '#e6edf3', fontFamily: 'monospace' }}>{b.latestCommitId}</strong>
                        </span>
                        <span>•</span>
                        <span className="branch-meta-item" style={{ maxWidth: '280px', overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap' }}>
                          "{b.latestCommitMsg}"
                        </span>
                      </div>
                    </div>
                  </div>

                  <div className="branch-card-actions">
                    <button
                      onClick={() => handleOpenInspector(b.name)}
                      className="btn-sm btn-outline"
                      title={`Inspect files inside data/branches/${b.name}/`}
                    >
                      <Eye size={14} /> View Files ({b.fileCount})
                    </button>

                    {isActive ? (
                      <span className="btn-sm" style={{ backgroundColor: 'rgba(56, 139, 253, 0.15)', color: 'var(--accent-blue)', border: '1px solid rgba(56, 139, 253, 0.35)', cursor: 'default' }}>
                        <Check size={14} /> Active
                      </span>
                    ) : (
                      <button
                        onClick={() => onSwitchBranch(b.name)}
                        className="btn-sm btn-primary"
                        title={`Switch active branch to ${b.name}`}
                      >
                        <ArrowRightCircle size={14} /> Checkout
                      </button>
                    )}

                    {!isActive && (
                      <button
                        onClick={() => {
                          if (window.confirm(`Are you sure you want to delete branch '${b.name}' and delete folder data/branches/${b.name}/?`)) {
                            onDeleteBranch(b.name);
                          }
                        }}
                        className="btn-icon"
                        style={{ color: 'var(--accent-red)', padding: '6px' }}
                        title={`Delete branch ${b.name}`}
                      >
                        <Trash2 size={16} />
                      </button>
                    )}
                  </div>
                </div>
              );
            })}
          </div>
        )}
      </div>

      {/* ================= MODAL: CREATE NEW BRANCH ================= */}
      {showCreateModal && (
        <div className="branch-modal-overlay">
          <div className="branch-modal-card">
            <div className="branch-modal-header">
              <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
                <GitBranch size={18} style={{ color: 'var(--accent-purple)' }} />
                <h3 style={{ margin: 0, fontSize: '1rem', fontWeight: 600 }}>Create New Branch</h3>
              </div>
              <button onClick={() => setShowCreateModal(false)} className="btn-icon">
                <X size={16} />
              </button>
            </div>

            <form onSubmit={handleCreateSubmit}>
              <div className="branch-modal-body">
                <div className="branch-form-group">
                  <label className="branch-form-label">
                    Branch Name
                  </label>
                  <input
                    type="text"
                    placeholder="e.g. feature-login, bugfix, dev"
                    value={newBranchName}
                    onChange={(e) => setNewBranchName(e.target.value.replace(/\s+/g, '-'))}
                    className="branch-form-input"
                    autoFocus
                    required
                  />
                  <div className="branch-form-hint">
                    A physical directory will be created at:{' '}
                    <code className="branch-meta-path">
                      data/branches/{newBranchName || '<name>'}/
                    </code>
                  </div>
                </div>

                <div className="branch-form-group">
                  <label className="branch-form-label">
                    Branch Off From (Source)
                  </label>
                  <select
                    value={sourceBranch}
                    onChange={(e) => setSourceBranch(e.target.value)}
                    className="branch-form-select"
                  >
                    {branches.map((b) => (
                      <option key={b.name} value={b.name}>
                        {b.name} ({b.fileCount} files)
                      </option>
                    ))}
                  </select>
                  <div className="branch-form-hint">
                    All snapshot files from <code style={{ color: 'var(--accent-purple)', fontFamily: 'monospace' }}>{sourceBranch}</code> will be cloned into the new branch folder.
                  </div>
                </div>

                <div style={{ display: 'flex', alignItems: 'center', gap: '8px', paddingTop: '6px' }}>
                  <input
                    type="checkbox"
                    id="switchNow"
                    checked={switchAfterCreate}
                    onChange={(e) => setSwitchAfterCreate(e.target.checked)}
                    style={{ cursor: 'pointer' }}
                  />
                  <label htmlFor="switchNow" style={{ fontSize: '0.82rem', color: 'var(--text-secondary)', cursor: 'pointer', userSelect: 'none' }}>
                    Switch working directory to this branch immediately
                  </label>
                </div>
              </div>

              <div className="branch-modal-footer">
                <button
                  type="button"
                  onClick={() => setShowCreateModal(false)}
                  className="btn-sm btn-outline"
                >
                  Cancel
                </button>
                <button type="submit" className="btn-sm btn-primary">
                  Create Branch
                </button>
              </div>
            </form>
          </div>
        </div>
      )}

      {/* ================= MODAL: BRANCH FILES INSPECTOR ================= */}
      {inspectingBranch && (
        <div className="branch-modal-overlay">
          <div className="branch-modal-card wide">
            <div className="branch-modal-header">
              <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
                <Folder size={18} style={{ color: 'var(--accent-purple)' }} />
                <h3 style={{ margin: 0, fontSize: '0.95rem', fontWeight: 600 }}>
                  Files in <span style={{ fontFamily: 'monospace', color: '#3fb950' }}>data/branches/{inspectingBranch}/</span>
                </h3>
              </div>
              <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
                <button
                  onClick={() => handleOpenInspector(inspectingBranch)}
                  className="btn-icon"
                  title="Reload files from disk"
                >
                  <RefreshCw size={14} />
                </button>
                <button onClick={() => setInspectingBranch(null)} className="btn-icon">
                  <X size={16} />
                </button>
              </div>
            </div>

            <div className="branch-modal-body" style={{ padding: '16px' }}>
              {loadingFiles ? (
                <div style={{ padding: '40px', textAlign: 'center', color: 'var(--text-muted)' }}>
                  Loading branch snapshot files...
                </div>
              ) : branchFiles.length === 0 ? (
                <div style={{ padding: '40px', textAlign: 'center', color: 'var(--text-muted)' }}>
                  <FileText size={32} style={{ margin: '0 auto 8px auto', opacity: 0.5 }} />
                  No files stored in this branch directory yet.
                </div>
              ) : (
                <div className="inspector-layout">
                  {/* Left Column: File List */}
                  <div className="inspector-files-list">
                    <div style={{ fontSize: '0.75rem', fontWeight: 600, color: 'var(--text-muted)', padding: '6px 8px', borderBottom: '1px solid var(--border-color)', marginBottom: '6px' }}>
                      SNAPSHOT FILES ({branchFiles.length})
                    </div>
                    {branchFiles.map((f) => (
                      <button
                        key={f.name}
                        onClick={() => handleOpenFileContent(inspectingBranch, f.name)}
                        className={`inspector-file-btn ${selectedFileName === f.name ? 'selected' : ''}`}
                      >
                        <span style={{ display: 'flex', alignItems: 'center', gap: '6px', overflow: 'hidden', textOverflow: 'ellipsis' }}>
                          <FileText size={13} style={{ flexShrink: 0 }} />
                          <span style={{ overflow: 'hidden', textOverflow: 'ellipsis' }}>{f.name}</span>
                        </span>
                        <span style={{ fontSize: '0.7rem', color: 'var(--text-muted)', flexShrink: 0 }}>{f.size} B</span>
                      </button>
                    ))}
                  </div>

                  {/* Right Column: File Content Preview */}
                  <div className="inspector-preview-box">
                    <div className="inspector-preview-header">
                      <span style={{ display: 'flex', alignItems: 'center', gap: '6px' }}>
                        <FileText size={14} style={{ color: '#3fb950' }} />
                        {selectedFileName || 'Select a file to inspect'}
                      </span>
                      <span style={{ fontSize: '0.72rem', color: 'var(--text-muted)' }}>Read-only Physical Snapshot</span>
                    </div>

                    <div className="inspector-code-content">
                      {selectedFileContent !== null ? (
                        selectedFileContent || <span style={{ color: 'var(--text-muted)', fontStyle: 'italic' }}>(Empty file)</span>
                      ) : (
                        <span style={{ color: 'var(--text-muted)', fontStyle: 'italic' }}>Select a file on the left to preview its content</span>
                      )}
                    </div>
                  </div>
                </div>
              )}
            </div>

            <div className="branch-modal-footer">
              <span style={{ fontSize: '0.78rem', color: 'var(--text-muted)', marginRight: 'auto' }}>
                Tip: To edit files on this branch, checkout this branch from the list.
              </span>
              <button
                type="button"
                onClick={() => setInspectingBranch(null)}
                className="btn-sm btn-outline"
              >
                Close
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
