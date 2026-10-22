#ifndef CONTRIBUTION_H
#define CONTRIBUTION_H

#include "commit.h"
#include <string>
#include <vector>

using namespace std;

// Holds daily commit count
class DayActivity {
public:
    string date;
    int count;

    DayActivity() : count(0) {}
    DayActivity(string d, int c) : date(d), count(c) {}
};

// Holds author contribution share
class AuthorStats {
public:
    string author;
    int commitCount;
    int percentage;

    AuthorStats() : commitCount(0), percentage(0) {}
};

// Summary metrics computed from MiniVCS commit history for GitHub-style Insights
class ContributionSummary {
public:
    int totalCommits;
    int activeDays;
    int currentStreak;
    int longestStreak;
    int activeContributorsCount;
    vector<DayActivity> days;
    vector<AuthorStats> authorBreakdown;

    ContributionSummary()
        : totalCommits(0), activeDays(0), currentStreak(0), longestStreak(0), activeContributorsCount(0) {}
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
