import React, { useState } from 'react';
import { GitBranch, Plus, Check, Trash2, Eye, Folder, FileText, ArrowRightCircle, RefreshCw, X, AlertCircle } from 'lucide-react';
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

  return (
    <div className="branches-container">
      {/* Branches Header */}
      <div className="panel-card mb-6">
        <div className="flex justify-between items-center flex-wrap gap-4">
          <div className="flex items-center gap-3">
            <div className="p-2.5 rounded-lg bg-purple-500/10 border border-purple-500/20 text-purple-400">
              <GitBranch size={22} />
            </div>
            <div>
              <div className="flex items-center gap-2">
                <h2 className="text-xl font-bold text-gray-100">Branches Module</h2>
                <span className="count-pill">{branches.length} branch{branches.length === 1 ? '' : 'es'}</span>
              </div>
              <p className="text-xs text-gray-400 mt-1">
                Each branch physically maintains its own files in{' '}
                <code className="text-emerald-400 font-mono">data/branches/&lt;name&gt;/</code>.
              </p>
            </div>
          </div>

          <div className="flex items-center gap-2">
            <button
              onClick={onRefresh}
              className="btn-sm btn-outline flex items-center gap-1.5"
              title="Refresh branches"
            >
              <RefreshCw size={14} /> Refresh
            </button>
            <button
              onClick={() => {
                setSourceBranch(currentBranch || 'main');
                setShowCreateModal(true);
              }}
              className="btn-sm btn-primary flex items-center gap-1.5"
            >
              <Plus size={15} /> New Branch
            </button>
          </div>
        </div>
      </div>

      {/* Default Branch Section */}
      <div className="panel-card mb-6">
        <div className="panel-header">
          <div className="flex items-center gap-2">
            <GitBranch size={16} className="text-emerald-400" />
            <h3 className="panel-title">Default Branch</h3>
            <span className="bg-emerald-500/15 text-emerald-400 text-xs px-2 py-0.5 rounded-full border border-emerald-500/30 font-medium">
              Default
            </span>
          </div>
          <span className="text-xs text-gray-400 font-mono">data/branches/main/</span>
        </div>

        <div className="branch-card default-branch-card p-4 rounded-lg bg-[#0d1117] border border-[#30363d] flex justify-between items-center flex-wrap gap-3">
          <div className="flex items-center gap-3">
            <div className="w-9 h-9 rounded-full bg-emerald-500/10 border border-emerald-500/30 flex items-center justify-center text-emerald-400 font-bold">
              <GitBranch size={16} />
            </div>
            <div>
              <div className="flex items-center gap-2">
                <span className="text-base font-semibold text-gray-100 font-mono">main</span>
                {defaultBranch.name === currentBranch && (
                  <span className="bg-blue-500/20 text-blue-400 text-xs px-2 py-0.5 rounded-full border border-blue-500/30 flex items-center gap-1">
                    <Check size={11} /> Checked Out (@{currentUser})
                  </span>
                )}
              </div>
              <div className="flex items-center gap-3 text-xs text-gray-400 mt-1">
                <span>Latest: <strong className="text-gray-300 font-mono">{defaultBranch.latestCommitId}</strong></span>
                <span>•</span>
                <span>Files: <strong className="text-purple-400">{defaultBranch.fileCount}</strong></span>
                <span>•</span>
                <span className="truncate max-w-xs">{defaultBranch.latestCommitMsg}</span>
              </div>
            </div>
          </div>

          <div className="flex items-center gap-2">
            <button
              onClick={() => handleOpenInspector('main')}
              className="btn-sm btn-outline flex items-center gap-1 text-xs"
              title="Inspect files in data/branches/main/"
            >
              <Eye size={13} /> View Files ({defaultBranch.fileCount})
            </button>
            {defaultBranch.name !== currentBranch && (
              <button
                onClick={() => onSwitchBranch('main')}
                className="btn-sm btn-primary flex items-center gap-1 text-xs"
              >
                <ArrowRightCircle size={13} /> Switch to main
              </button>
            )}
          </div>
        </div>
      </div>

      {/* Other Branches Section */}
      <div className="panel-card">
        <div className="panel-header">
          <div className="flex items-center gap-2">
            <Folder size={16} className="text-purple-400" />
            <h3 className="panel-title">Feature & Team Branches ({otherBranches.length})</h3>
          </div>
          <span className="text-xs text-gray-400">Isolated workspace folders</span>
        </div>

        {otherBranches.length === 0 ? (
          <div className="empty-hint p-8 text-center text-gray-400">
            <GitBranch size={32} className="mx-auto mb-2 text-gray-600" />
            <p className="text-sm font-medium">No custom branches created yet.</p>
            <p className="text-xs text-gray-500 mt-1">
              Click <strong>"New Branch"</strong> to create an isolated branch with its own physical files.
            </p>
          </div>
        ) : (
          <div className="branch-grid space-y-3">
            {otherBranches.map((b) => {
              const isActive = b.name === currentBranch;
              return (
                <div
                  key={b.name}
                  className={`branch-row p-4 rounded-lg bg-[#0d1117] border ${
                    isActive ? 'border-purple-500/50 bg-purple-500/5' : 'border-[#30363d]'
                  } flex justify-between items-center flex-wrap gap-3 transition-all`}
                >
                  <div className="flex items-center gap-3">
                    <div
                      className={`w-9 h-9 rounded-full flex items-center justify-center font-bold ${
                        isActive
                          ? 'bg-purple-500/20 border border-purple-500/40 text-purple-400'
                          : 'bg-gray-800 border border-gray-700 text-gray-400'
                      }`}
                    >
                      <GitBranch size={16} />
                    </div>
                    <div>
                      <div className="flex items-center gap-2">
                        <span className="text-base font-semibold text-gray-100 font-mono">{b.name}</span>
                        {isActive && (
                          <span className="bg-purple-500/20 text-purple-400 text-xs px-2 py-0.5 rounded-full border border-purple-500/30 flex items-center gap-1">
                            <Check size={11} /> Checked Out (@{currentUser})
                          </span>
                        )}
                      </div>
                      <div className="flex items-center gap-3 text-xs text-gray-400 mt-1 flex-wrap">
                        <span>Branched from: <strong className="text-emerald-400 font-mono">{b.parentBranch || 'main'}</strong></span>
                        <span>•</span>
                        <span>Folder: <code className="text-gray-300">data/branches/{b.name}/</code></span>
                        <span>•</span>
                        <span>Files: <strong className="text-purple-400">{b.fileCount}</strong></span>
                        <span>•</span>
                        <span>Commit: <strong className="text-gray-300 font-mono">{b.latestCommitId}</strong></span>
                        <span>•</span>
                        <span className="truncate max-w-xs">{b.latestCommitMsg}</span>
                      </div>
                    </div>
                  </div>

                  <div className="flex items-center gap-2">
                    <button
                      onClick={() => handleOpenInspector(b.name)}
                      className="btn-sm btn-outline flex items-center gap-1 text-xs"
                      title={`Inspect files in data/branches/${b.name}/`}
                    >
                      <Eye size={13} /> View Files ({b.fileCount})
                    </button>

                    {isActive ? (
                      <span className="btn-sm bg-purple-500/10 text-purple-400 border border-purple-500/30 text-xs flex items-center gap-1 cursor-default">
                        <Check size={13} /> Active
                      </span>
                    ) : (
                      <button
                        onClick={() => onSwitchBranch(b.name)}
                        className="btn-sm btn-primary flex items-center gap-1 text-xs"
                        title={`Switch working directory to ${b.name}`}
                      >
                        <ArrowRightCircle size={13} /> Checkout
                      </button>
                    )}

                    {!isActive && (
                      <button
                        onClick={() => {
                          if (window.confirm(`Are you sure you want to delete branch '${b.name}' and remove data/branches/${b.name}/?`)) {
                            onDeleteBranch(b.name);
                          }
                        }}
                        className="btn-sm btn-icon text-red-400 hover:text-red-300 hover:bg-red-500/10"
                        title={`Delete branch ${b.name}`}
                      >
                        <Trash2 size={14} />
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
        <div className="modal-overlay">
          <div className="modal-dialog max-w-md">
            <div className="modal-header">
              <div className="flex items-center gap-2">
                <GitBranch size={18} className="text-purple-400" />
                <h3 className="modal-title">Create New Branch</h3>
              </div>
              <button onClick={() => setShowCreateModal(false)} className="btn-sm btn-icon">
                <X size={16} />
              </button>
            </div>

            <form onSubmit={handleCreateSubmit} className="modal-body space-y-4">
              <div>
                <label className="block text-xs font-semibold text-gray-300 mb-1">
                  Branch Name
                </label>
                <input
                  type="text"
                  placeholder="e.g. feature-login, bugfix-auth, dev"
                  value={newBranchName}
                  onChange={(e) => setNewBranchName(e.target.value.replace(/\s+/g, '-'))}
                  className="input-text w-full font-mono"
                  autoFocus
                  required
                />
                <p className="text-[11px] text-gray-400 mt-1">
                  A physical directory will be created at:{' '}
                  <code className="text-emerald-400 font-mono">
                    data/branches/{newBranchName || '<name>'}/
                  </code>
                </p>
              </div>

              <div>
                <label className="block text-xs font-semibold text-gray-300 mb-1">
                  Branch From (Source)
                </label>
                <select
                  value={sourceBranch}
                  onChange={(e) => setSourceBranch(e.target.value)}
                  className="select-input w-full font-mono"
                >
                  {branches.map((b) => (
                    <option key={b.name} value={b.name}>
                      {b.name} ({b.fileCount} files)
                    </option>
                  ))}
                </select>
                <p className="text-[11px] text-gray-400 mt-1">
                  All files from <code className="text-purple-400 font-mono">{sourceBranch}</code> will be copied into the new branch folder.
                </p>
              </div>

              <div className="flex items-center gap-2 pt-2">
                <input
                  type="checkbox"
                  id="switchNow"
                  checked={switchAfterCreate}
                  onChange={(e) => setSwitchAfterCreate(e.target.checked)}
                  className="rounded border-gray-700 text-purple-600 focus:ring-purple-500"
                />
                <label htmlFor="switchNow" className="text-xs text-gray-300 select-none cursor-pointer">
                  Switch to this branch immediately after creation
                </label>
              </div>

              <div className="modal-footer pt-4 border-t border-[#30363d] flex justify-end gap-2">
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
        <div className="modal-overlay">
          <div className="modal-dialog max-w-3xl">
            <div className="modal-header">
              <div className="flex items-center gap-2">
                <Folder size={18} className="text-purple-400" />
                <h3 className="modal-title">
                  Files in <span className="font-mono text-emerald-400">data/branches/{inspectingBranch}/</span>
                </h3>
              </div>
              <div className="flex items-center gap-2">
                <button
                  onClick={() => handleOpenInspector(inspectingBranch)}
                  className="btn-sm btn-icon"
                  title="Reload files from disk"
                >
                  <RefreshCw size={14} />
                </button>
                <button onClick={() => setInspectingBranch(null)} className="btn-sm btn-icon">
                  <X size={16} />
                </button>
              </div>
            </div>

            <div className="modal-body p-4">
              {loadingFiles ? (
                <div className="p-8 text-center text-gray-400">Loading branch files...</div>
              ) : branchFiles.length === 0 ? (
                <div className="empty-hint p-8 text-center text-gray-400">
                  <FileText size={28} className="mx-auto mb-2 text-gray-600" />
                  No files stored in this branch folder yet.
                </div>
              ) : (
                <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
                  {/* File List Column */}
                  <div className="border border-[#30363d] rounded-lg bg-[#0d1117] p-2 space-y-1">
                    <div className="text-xs font-semibold text-gray-400 px-2 py-1 border-b border-[#30363d] mb-1">
                      Files ({branchFiles.length})
                    </div>
                    {branchFiles.map((f) => (
                      <button
                        key={f.name}
                        onClick={() => handleOpenFileContent(inspectingBranch, f.name)}
                        className={`w-full text-left px-2.5 py-1.5 rounded text-xs flex items-center justify-between font-mono ${
                          selectedFileName === f.name
                            ? 'bg-purple-500/20 text-purple-300 font-semibold border border-purple-500/30'
                            : 'text-gray-300 hover:bg-[#161b22]'
                        }`}
                      >
                        <span className="flex items-center gap-1.5 truncate">
                          <FileText size={13} className="text-gray-400 flex-shrink-0" />
                          <span className="truncate">{f.name}</span>
                        </span>
                        <span className="text-[10px] text-gray-500 flex-shrink-0">{f.size} B</span>
                      </button>
                    ))}
                  </div>

                  {/* File Content Preview Column */}
                  <div className="md:col-span-2 border border-[#30363d] rounded-lg bg-[#0d1117] p-3 flex flex-col">
                    <div className="flex justify-between items-center border-b border-[#30363d] pb-2 mb-2">
                      <span className="text-xs font-mono font-semibold text-gray-200 flex items-center gap-1.5">
                        <FileText size={14} className="text-emerald-400" />
                        {selectedFileName || 'Select a file to inspect'}
                      </span>
                      <span className="text-[10px] text-gray-500">Read-only Snapshot</span>
                    </div>

                    <div className="flex-1 overflow-auto max-h-64 bg-[#161b22] p-3 rounded font-mono text-xs text-gray-200 whitespace-pre">
                      {selectedFileContent !== null ? (
                        selectedFileContent || <span className="text-gray-500 italic">(Empty file)</span>
                      ) : (
                        <span className="text-gray-500 italic">Select a file on the left to preview content</span>
                      )}
                    </div>
                  </div>
                </div>
              )}
            </div>

            <div className="modal-footer pt-3 border-t border-[#30363d] flex justify-between items-center">
              <span className="text-xs text-gray-400">
                To edit files on this branch, switch to it from the branches list.
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

