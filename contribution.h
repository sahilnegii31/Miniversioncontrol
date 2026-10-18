#ifndef CONTRIBUTION_H
#define CONTRIBUTION_H

#include "commit.h"
#include <string>
#include <vector>

using namespace std;

// Holds daily commit count
struct DayActivity {
    string date;
    int count;
};

// Holds author contribution share
struct AuthorStats {
    string author;
    int commitCount;
    int percentage;
};

// Summary metrics computed from MiniVCS commit history for GitHub-style Insights
struct ContributionSummary {
    int totalCommits;
    int activeDays;
    int currentStreak;
    int longestStreak;
    int activeContributorsCount;
    vector<DayActivity> days;
    vector<AuthorStats> authorBreakdown;
};

// Calculate contribution statistics from the commit history
ContributionSummary calculateContributions(CommitNode* head, const string& authorFilter = "");

// Export contribution summary as JSON with embedded C++ SVG charts (Commits Histogram & Contributor Breakdown)
string contributionsToJson(const ContributionSummary& summary);

// Generate SVG Commit Activity Histogram directly from C++
string generateContributionSvg(const ContributionSummary& summary);

// Generate SVG Contributor Contribution Share directly from C++
string generateContributorSvg(const ContributionSummary& summary);

#endif // CONTRIBUTION_H
