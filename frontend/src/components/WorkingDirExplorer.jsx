import React, { useState } from 'react';
import { Folder, FileText, Plus, Save, Trash2, ArrowRightCircle, RefreshCw, GitBranch } from 'lucide-react';

export default function WorkingDirExplorer({
  user,
  currentBranch = 'main',
  files,
  onRefresh,
  onStageFile,
  onSaveFile,
  onDeleteFile
}) {
  const [selectedFile, setSelectedFile] = useState(null);
  const [fileContent, setFileContent] = useState('');
  const [isEditing, setIsEditing] = useState(false);
  const [newFileName, setNewFileName] = useState('');
  const [showNewModal, setShowNewModal] = useState(false);
  const [statusMsg, setStatusMsg] = useState('');

  const handleSelectFile = async (f) => {
    setSelectedFile(f.name);
    try {
      const res = await fetch(`/api/file?user=${encodeURIComponent(user)}&name=${encodeURIComponent(f.name)}`);
      const data = await res.json();
      setFileContent(data.content || '');
      setIsEditing(true);
    } catch {
      setStatusMsg('Failed to load file');
    }
  };

  const handleSave = async () => {
    if (!selectedFile) return;
    try {
      await onSaveFile(selectedFile, fileContent);
      setStatusMsg(`Saved ${selectedFile} successfully!`);
      setTimeout(() => setStatusMsg(''), 3000);
    } catch (err) {
      alert(`Save failed: ${err.message || 'Server connection error'}`);
    }
  };

  const handleCreateNew = async (e) => {
    e.preventDefault();
    const clean = newFileName.trim();
    if (!clean) return;
    try {
      await onSaveFile(clean, '');
      setSelectedFile(clean);
      setFileContent('');
      setIsEditing(true);
      setNewFileName('');
      setShowNewModal(false);
      if (onRefresh) onRefresh();
    } catch (err) {
      alert(`Could not create file: ${err.message || 'Check if C++ backend is running'}`);
    }
  };

  return (
    <div className="panel-card">
      <div className="panel-header">
        <div className="flex-center gap-2">
          <Folder size={18} className="text-blue-400" />
          <h3 className="panel-title">Working Directory</h3>
          <span className="user-badge font-mono">data/working/{user}</span>
          <span className="user-badge font-mono text-purple-300 bg-purple-500/10 border border-purple-500/30 flex items-center gap-1">
            <GitBranch size={12} /> {currentBranch}
          </span>
        </div>
        <div className="flex-center gap-2">
          <button onClick={() => setShowNewModal(true)} className="btn-sm btn-outline">
            <Plus size={14} /> New File
          </button>
          <button onClick={onRefresh} className="btn-sm btn-icon" title="Refresh files">
            <RefreshCw size={14} />
          </button>
        </div>
      </div>

      <div className="workspace-body">
        {/* File List Column */}
        <div className="file-list-col">
          <div className="file-list-header">Files ({files.length})</div>
          {files.length === 0 ? (
            <div className="empty-hint">No files found. Create one using "New File"!</div>
          ) : (
            <div className="file-items-scroll">
              {files.map((f) => (
                <div
                  key={f.name}
                  className={`file-item ${selectedFile === f.name ? 'selected' : ''}`}
                  onClick={() => handleSelectFile(f)}
                >
                  <div className="file-item-left">
                    <FileText size={16} className="text-gray-400" />
                    <span className="file-name">{f.name}</span>
                  </div>
                  <div className="file-actions">
                    <button
                      onClick={(e) => {
                        e.stopPropagation();
                        onStageFile(f.name);
                      }}
                      className="btn-action-stage"
                      title="Stage file (git add)"
                    >
                      <ArrowRightCircle size={14} /> Stage
                    </button>
                    <button
                      onClick={(e) => {
                        e.stopPropagation();
                        if (confirm(`Delete ${f.name}?`)) onDeleteFile(f.name);
                      }}
                      className="btn-action-delete"
                      title="Delete file"
                    >
                      <Trash2 size={13} />
                    </button>
                  </div>
                </div>
              ))}
            </div>
          )}
        </div>

        {/* File Editor Column */}
        <div className="file-editor-col">
          {isEditing && selectedFile ? (
            <div className="editor-container">
              <div className="editor-top-bar">
                <span className="editor-filename">Editing: {selectedFile}</span>
                <div className="flex-center gap-2">
                  {statusMsg && <span className="status-badge">{statusMsg}</span>}
                  <button onClick={handleSave} className="btn-sm btn-primary">
                    <Save size={14} /> Save Changes
                  </button>
                  <button
                    onClick={() => onStageFile(selectedFile)}
                    className="btn-sm btn-emerald"
                  >
                    <ArrowRightCircle size={14} /> Save & Stage
                  </button>
                </div>
              </div>
              <textarea
                value={fileContent}
                onChange={(e) => setFileContent(e.target.value)}
                className="code-editor"
                placeholder="Type file content here..."
                rows={12}
              />
            </div>
          ) : (
            <div className="editor-placeholder">
              <FileText size={36} className="text-gray-600 mb-2" />
              <p>Select a file from the list or create a new file to edit</p>
            </div>
          )}
        </div>
      </div>

      {/* New File Modal */}
      {showNewModal && (
        <div className="modal-backdrop">
          <div className="modal-box">
            <h4 className="modal-title">Create New File</h4>
            <form onSubmit={handleCreateNew}>
              <input
                type="text"
                value={newFileName}
                onChange={(e) => setNewFileName(e.target.value)}
                placeholder="e.g. notes.txt, main.py"
                className="modal-input"
                autoFocus
              />
              <div className="modal-btn-row">
                <button
                  type="button"
                  onClick={() => setShowNewModal(false)}
                  className="btn-sm btn-outline"
                >
                  Cancel
                </button>
                <button type="submit" className="btn-sm btn-primary">
                  Create File
                </button>
              </div>
            </form>
          </div>
        </div>
      )}
    </div>
  );
}

