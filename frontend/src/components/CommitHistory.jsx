import React, { useState } from 'react';
import { GitCommit, GitBranch, Clock, User, Eye, RotateCcw, ArrowDownCircle, X } from 'lucide-react';

export default function CommitHistory({
  commits,
  onCheckout,
  onUndoLast,
  onRevertTo
}) {
  const [selectedCommitDiff, setSelectedCommitDiff] = useState(null);
  const [loadingDiff, setLoadingDiff] = useState(false);

  const handleInspect = async (commitId) => {
    setLoadingDiff(true);
    try {
      const res = await fetch(`/api/commit/diff?commitId=${encodeURIComponent(commitId)}`);
      const data = await res.json();
      setSelectedCommitDiff(data);
    } catch {
      alert('Failed to load commit diff');
    } finally {
      setLoadingDiff(false);
    }
  };

  return (
    <div className="panel-card">
      <div className="panel-header">
        <div className="flex-center gap-2">
          <GitCommit size={18} className="text-purple-400" />
          <h3 className="panel-title">Commit History & Version Control</h3>
          <span className="count-pill">{commits.length} commit(s)</span>
        </div>
        <div className="flex-center gap-2">
          {commits.length > 0 && (
            <button
              onClick={onUndoLast}
              className="btn-sm btn-outline text-amber-400"
              title="Undo last commit (git reset HEAD~1)"
            >
              <RotateCcw size={14} /> Undo Last Commit
            </button>
          )}
        </div>
      </div>

      <div className="history-body">
        {commits.length === 0 ? (
          <div className="empty-hint">
            No commits found in history. Make your first commit above to see it here and record activity on the contribution chart!
          </div>
        ) : (
          <div className="timeline-container">
            {commits.map((c, idx) => (
              <div key={c.commitId} className="timeline-node">
                <div className="timeline-marker">
                  <div className="timeline-dot" />
                  {idx !== commits.length - 1 && <div className="timeline-line" />}
                </div>

                <div className="timeline-card">
                  <div className="timeline-card-header">
                    <div className="flex-center gap-2">
                      <span className="commit-id-pill">{c.commitId}</span>
                      <span className="commit-branch-tag">
                        <GitBranch size={12} /> {c.branch}
                      </span>
                      {c.parentId && c.parentId !== 'NONE' && (
                        <span className="parent-pill">parent: {c.parentId}</span>
                      )}
                    </div>
                    <div className="timeline-timestamp">
                      <Clock size={12} /> {c.timestamp}
                    </div>
                  </div>

                  <p className="commit-message-text">{c.message}</p>

                  <div className="timeline-card-footer">
                    <div className="author-tag">
                      <User size={12} /> {c.author}
                    </div>

                    <div className="flex-center gap-2">
                      <button
                        onClick={() => handleInspect(c.commitId)}
                        className="btn-xs btn-outline"
                        title="View file changes"
                      >
                        <Eye size={12} /> Inspect Changes
                      </button>
                      <button
                        onClick={() => onCheckout(c.commitId)}
                        className="btn-xs btn-outline text-blue-400"
                        title="Checkout / move working directory to this commit"
                      >
                        <ArrowDownCircle size={12} /> Checkout
                      </button>
                      <button
                        onClick={() => onRevertTo(c.commitId)}
                        className="btn-xs btn-outline text-rose-400"
                        title="Revert history back to this commit"
                      >
                        <RotateCcw size={12} /> Revert To
                      </button>
                    </div>
                  </div>
                </div>
              </div>
            ))}
          </div>
        )}
      </div>

      {/* Commit Diff Modal */}
      {selectedCommitDiff && (
        <div className="modal-backdrop">
          <div className="modal-box modal-large">
            <div className="modal-header">
              <div className="flex-center gap-2">
                <GitCommit size={18} className="text-purple-400" />
                <h4 className="modal-title">
                  Commit Details: {selectedCommitDiff.commitId} ({selectedCommitDiff.message})
                </h4>
              </div>
              <button
                onClick={() => setSelectedCommitDiff(null)}
                className="btn-icon"
              >
                <X size={18} />
              </button>
            </div>

            <div className="diff-meta-bar">
              <div><strong>Author:</strong> {selectedCommitDiff.author}</div>
              <div><strong>Timestamp:</strong> {selectedCommitDiff.timestamp}</div>
              <div><strong>Branch:</strong> {selectedCommitDiff.branch}</div>
              <div><strong>Parent:</strong> {selectedCommitDiff.parentId}</div>
            </div>

            <div className="diff-files-list">
              <h5 className="diff-files-title">
                Snapshot Files ({selectedCommitDiff.files?.length || 0})
              </h5>

              {selectedCommitDiff.files?.length === 0 ? (
                <div className="empty-hint">No snapshot files in this commit.</div>
              ) : (
                selectedCommitDiff.files?.map((file, i) => (
                  <div key={i} className="diff-file-card">
                    <div className="diff-file-header">
                      <span className="diff-filename">{file.filename}</span>
                      <span className={`status-tag status-${file.status.toLowerCase()}`}>
                        {file.status}
                      </span>
                    </div>

                    <div className="diff-content-grid">
                      {file.parentContent !== undefined && file.status === 'MODIFIED' && (
                        <div className="diff-pane">
                          <div className="pane-title">Parent Version ({selectedCommitDiff.parentId})</div>
                          <pre className="pane-code deleted">{file.parentContent || '(empty)'}</pre>
                        </div>
                      )}
                      <div className="diff-pane">
                        <div className="pane-title">This Commit Version ({selectedCommitDiff.commitId})</div>
                        <pre className="pane-code added">{file.currentContent || '(empty)'}</pre>
                      </div>
                    </div>
                  </div>
                ))
              )}
            </div>
          </div>
        </div>
      )}
    </div>
  );
}

