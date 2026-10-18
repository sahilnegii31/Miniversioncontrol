import React, { useState } from 'react';
import { Activity, GitCommit, Users, Flame, Trophy, Calendar, FileText, CheckCircle2 } from 'lucide-react';

export default function InsightsPage({
  contributions,
  commits,
  files,
  selectedAuthor,
  onSelectAuthor,
  isLive
}) {
  // Navigation between the 3 GitHub Insights columns: 'overview', 'pulse', 'commits', 'contributors'
  const [activeSubTab, setActiveSubTab] = useState('overview');

  if (!contributions) {
    return (
      <div className="bg-[#161b22] border border-[#30363d] rounded-xl p-8 text-center text-gray-400">
        <Activity className="animate-spin mx-auto mb-2 text-emerald-400" size={24} />
        Loading MiniVCS Insights from C++ Engine...
      </div>
    );
  }

  const {
    totalCommits,
    activeDays,
    currentStreak,
    longestStreak,
    activeContributorsCount,
    authorBreakdown,
    chartSvg,
    contributorSvg
  } = contributions;

  return (
    <div className="insights-container">
      {/* Top Insights Header & Sub-Navigation */}
      <div className="insights-header">
        <div className="insights-title-box">
          <div className="icon-badge">
            <Activity className="icon-pulse text-emerald-400" size={20} />
          </div>
          <div>
            <h2 className="title-text">Repository Insights</h2>
            <p className="subtitle-text">
              Real-time analytics & charts powered by C++ Engine
              {isLive && <span className="live-pill"><span className="live-dot" /> LIVE C++</span>}
            </p>
          </div>
        </div>

        {/* GitHub-style Insights Column Switcher */}
        <div className="insights-subnav">
          <button
            onClick={() => setActiveSubTab('overview')}
            className={`subnav-btn ${activeSubTab === 'overview' ? 'active' : ''}`}
          >
            All 3 Columns
          </button>
          <button
            onClick={() => setActiveSubTab('pulse')}
            className={`subnav-btn ${activeSubTab === 'pulse' ? 'active' : ''}`}
          >
            <Activity size={14} /> 1. Pulse
          </button>
          <button
            onClick={() => setActiveSubTab('commits')}
            className={`subnav-btn ${activeSubTab === 'commits' ? 'active' : ''}`}
          >
            <GitCommit size={14} /> 2. Commits
          </button>
          <button
            onClick={() => setActiveSubTab('contributors')}
            className={`subnav-btn ${activeSubTab === 'contributors' ? 'active' : ''}`}
          >
            <Users size={14} /> 3. Contributors
          </button>
        </div>
      </div>

      {/* Main Content: The 3 Insights Columns */}
      <div className="insights-body">
        {/* ================= COLUMN 1: PULSE ================= */}
        {(activeSubTab === 'overview' || activeSubTab === 'pulse') && (
          <div className="insight-section">
            <div className="insight-section-header">
              <div className="flex-center gap-2">
                <Activity size={18} className="text-emerald-400" />
                <h3 className="section-title">Pulse</h3>
                <span className="badge-tag">Repository Overview</span>
              </div>
            </div>

            <div className="pulse-grid">
              <div className="stat-card">
                <div className="stat-header">
                  <span className="stat-label">Total Commits</span>
                  <GitCommit size={16} className="text-emerald-400" />
                </div>
                <div className="stat-val">{totalCommits}</div>
                <div className="stat-sub">recorded in VCS history</div>
              </div>

              <div className="stat-card">
                <div className="stat-header">
                  <span className="stat-label">Active Contributors</span>
                  <Users size={16} className="text-blue-400" />
                </div>
                <div className="stat-val text-blue-400">{activeContributorsCount || authorBreakdown?.length || 0}</div>
                <div className="stat-sub">registered team members</div>
              </div>

              <div className="stat-card">
                <div className="stat-header">
                  <span className="stat-label">Active Streak</span>
                  <Flame size={16} className="text-orange-400" />
                </div>
                <div className="stat-val text-orange-400">{currentStreak} <span className="stat-unit">days</span></div>
                <div className="stat-sub">record: {longestStreak} days</div>
              </div>

              <div className="stat-card">
                <div className="stat-header">
                  <span className="stat-label">Tracked Files</span>
                  <FileText size={16} className="text-purple-400" />
                </div>
                <div className="stat-val text-purple-400">{files?.length || 0}</div>
                <div className="stat-sub">in active working directory</div>
              </div>
            </div>

            {/* Pulse Activity Stream */}
            <div className="pulse-recent-box">
              <h4 className="box-subtitle">Recent Commit Pulse</h4>
              {commits.length === 0 ? (
                <div className="empty-hint text-xs">No commits recorded yet.</div>
              ) : (
                <div className="recent-commit-list">
                  {commits.slice(-4).reverse().map((c) => (
                    <div key={c.commitId} className="recent-commit-row">
                      <div className="flex-center gap-2">
                        <span className="commit-id-pill">{c.commitId}</span>
                        <span className="recent-msg">{c.message}</span>
                      </div>
                      <div className="recent-meta">
                        <span>by <strong>{c.author}</strong></span>
                        <span>•</span>
                        <span>{c.timestamp}</span>
                      </div>
                    </div>
                  ))}
                </div>
              )}
            </div>
          </div>
        )}

        {/* ================= COLUMN 2: COMMITS (C++ HISTOGRAM CHART) ================= */}
        {(activeSubTab === 'overview' || activeSubTab === 'commits') && (
          <div className="insight-section">
            <div className="insight-section-header">
              <div className="flex-center gap-2">
                <GitCommit size={18} className="text-purple-400" />
                <h3 className="section-title">Commits Histogram</h3>
                <span className="badge-tag text-emerald-400">⚡ C++ Vector Chart Library</span>
              </div>

              {/* Author Filter for Commits */}
              <div className="filter-select-wrapper">
                <span className="filter-label">Filter Author:</span>
                <select
                  value={selectedAuthor}
                  onChange={(e) => onSelectAuthor(e.target.value)}
                  className="select-input"
                >
                  <option value="">All Authors</option>
                  {authorBreakdown && authorBreakdown.map((ab) => (
                    <option key={ab.author} value={ab.author}>
                      {ab.author} ({ab.commitCount} commits)
                    </option>
                  ))}
                </select>
              </div>
            </div>

            <div className="chart-render-wrapper">
              <div className="chart-info-banner">
                <span>
                  Real-time histogram generated natively by C++ library (<code>c_chart.cpp</code>).
                  Every time you commit, the bar height increases in real time!
                </span>
              </div>

              {chartSvg ? (
                <div
                  className="cpp-svg-wrapper"
                  dangerouslySetInnerHTML={{ __html: chartSvg }}
                />
              ) : (
                <div className="cpp-svg-wrapper">
                  <img
                    src={`/api/contributions/svg?user=${encodeURIComponent(selectedAuthor)}&t=${Date.now()}`}
                    alt="Live C++ Generated Commit Histogram"
                    className="w-full"
                  />
                </div>
              )}
            </div>
          </div>
        )}

        {/* ================= COLUMN 3: CONTRIBUTORS ================= */}
        {(activeSubTab === 'overview' || activeSubTab === 'contributors') && (
          <div className="insight-section">
            <div className="insight-section-header">
              <div className="flex-center gap-2">
                <Users size={18} className="text-blue-400" />
                <h3 className="section-title">Contributors</h3>
                <span className="badge-tag">Share Breakdown</span>
              </div>
            </div>

            {/* Contributor SVG Chart generated by C++ */}
            {contributorSvg && (
              <div className="mb-4">
                <div
                  className="cpp-svg-wrapper"
                  dangerouslySetInnerHTML={{ __html: contributorSvg }}
                />
              </div>
            )}

            {/* Contributor Cards List */}
            <div className="contributors-cards-grid">
              {authorBreakdown && authorBreakdown.length > 0 ? (
                authorBreakdown.map((ab, idx) => {
                  const colors = ['#2ea043', '#388bfd', '#f0883e', '#a371f7'];
                  const color = colors[idx % colors.length];
                  return (
                    <div key={ab.author} className="contributor-card">
                      <div className="contributor-avatar" style={{ backgroundColor: color }}>
                        {ab.author.charAt(0).toUpperCase()}
                      </div>
                      <div className="contributor-info">
                        <div className="contributor-name">{ab.author}</div>
                        <div className="contributor-stats">
                          <strong>{ab.commitCount}</strong> commit{ab.commitCount === 1 ? '' : 's'}
                          <span className="bullet-sep">•</span>
                          <span style={{ color }}>{ab.percentage}% of project</span>
                        </div>
                        <div className="progress-track">
                          <div
                            className="progress-fill"
                            style={{ width: `${ab.percentage}%`, backgroundColor: color }}
                          />
                        </div>
                      </div>
                    </div>
                  );
                })
              ) : (
                <div className="empty-hint text-xs">No contributor statistics available yet.</div>
              )}
            </div>
          </div>
        )}
      </div>
    </div>
  );
}

