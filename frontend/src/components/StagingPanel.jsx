import React, { useState, useEffect } from 'react';
import { Layers, GitCommit, Undo, Trash2, CheckCircle2, AlertCircle } from 'lucide-react';

export default function StagingPanel({
  stagedFiles,
  onUnstage,
  onClearStaging,
  onCommit,
  currentBranch = 'main'
}) {
  const [commitMsg, setCommitMsg] = useState('');
  const [branch, setBranch] = useState(currentBranch || 'main');
  const [isCommitting, setIsCommitting] = useState(false);
  const [error, setError] = useState('');

  useEffect(() => {
    if (currentBranch) {
      setBranch(currentBranch);
    }
  }, [currentBranch]);

  const handleCommitSubmit = async (e) => {
    e.preventDefault();
    if (!commitMsg.trim()) {
      setError('Please provide a commit message!');
      return;
    }
    if (stagedFiles.length === 0) {
      setError('No files in staging area! Stage files first.');
      return;
    }

    setError('');
    setIsCommitting(true);
    try {
      await onCommit(commitMsg.trim(), branch.trim());
      setCommitMsg('');
    } catch (err) {
      setError(err.message || 'Failed to commit');
    } finally {
      setIsCommitting(false);
    }
  };

  return (
    <div className="panel-card">
      <div className="panel-header">
        <div className="flex-center gap-2">
          <Layers size={18} className="text-emerald-400" />
          <h3 className="panel-title">Staging Area</h3>
          <span className="count-pill">{stagedFiles.length} file(s) staged</span>
        </div>
        <div className="flex-center gap-2">
          {stagedFiles.length > 0 && (
            <>
              <button
                onClick={onUnstage}
                className="btn-sm btn-outline text-amber-400"
                title="Unstage top of stack"
              >
                <Undo size={14} /> Unstage Last
              </button>
              <button
                onClick={onClearStaging}
                className="btn-sm btn-outline text-red-400"
                title="Clear staging"
              >
                <Trash2 size={14} /> Clear
              </button>
            </>
          )}
        </div>
      </div>

      <div className="staging-body">
        {/* Staged files listing */}
        <div className="staged-list-area">
          {stagedFiles.length === 0 ? (
            <div className="empty-staging">
              <CheckCircle2 size={32} className="text-gray-600 mb-2" />
              <p className="text-gray-400 text-sm">Staging area is empty.</p>
              <p className="text-gray-500 text-xs">Stage files from the Working Directory on the left to prepare a commit.</p>
            </div>
          ) : (
            <div className="staged-items">
              {stagedFiles.map((f, idx) => (
                <div key={idx} className="staged-item">
                  <div className="staged-item-header">
                    <span className="staged-item-name">{f.filename}</span>
                    <span className="staged-badge">STAGED</span>
                  </div>
                  <pre className="staged-content-preview">{f.content || '(empty file)'}</pre>
                </div>
              ))}
            </div>
          )}
        </div>

        {/* Commit Form */}
        <div className="commit-box">
          <h4 className="commit-title">
            <GitCommit size={16} className="text-emerald-400" /> Create Commit
          </h4>

          {error && (
            <div className="commit-error">
              <AlertCircle size={14} /> {error}
            </div>
          )}

          <form onSubmit={handleCommitSubmit} className="commit-form">
            <div className="form-group">
              <label className="form-label">Branch</label>
              <input
                type="text"
                value={branch}
                onChange={(e) => setBranch(e.target.value)}
                className="form-input"
                placeholder="main"
              />
            </div>

            <div className="form-group">
              <label className="form-label">Commit Message</label>
              <textarea
                value={commitMsg}
                onChange={(e) => setCommitMsg(e.target.value)}
                className="form-textarea"
                placeholder="Describe your changes..."
                rows={3}
              />
            </div>

            <button
              type="submit"
              disabled={isCommitting || stagedFiles.length === 0}
              className={`btn-commit ${stagedFiles.length === 0 ? 'disabled' : ''}`}
            >
              <GitCommit size={16} />
              {isCommitting ? 'Committing in C++...' : `Commit ${stagedFiles.length} file(s)`}
            </button>
          </form>
        </div>
      </div>
    </div>
  );
}

