#ifndef C_CHART_H
#define C_CHART_H

#include <string>
#include <vector>

namespace cchart {

// Holds daily commit count for the histogram
struct DailyCommit {
    std::string date; // "YYYY-MM-DD" or "MM-DD"
    int count;        // Number of commits
};

// Holds contributor commit share
struct ContributorShare {
    std::string author;
    int count;
    int percentage;
};

// Standalone C++ Vector Chart Library for MiniVCS Insights
class CommitChartGenerator {
private:
    std::string title;
    int totalCommits;
    int currentStreak;
    int activeDays;
    std::vector<DailyCommit> dailyCommits;
    std::vector<ContributorShare> contributors;

public:
    CommitChartGenerator();

    void setTitle(const std::string& t);
    void setStats(int total, int streak, int days);
    void addDailyData(const std::string& date, int count);
    void addContributor(const std::string& author, int count, int percentage);
    void clear();

    // 1. Renders the Commit Histogram (for Insights -> Commits)
    std::string renderCommitHistogramSvg() const;

    // 2. Renders the Contributor Share Chart (for Insights -> Contributors)
    std::string renderContributorChartSvg() const;

    // 3. Combined Overview Chart
    std::string renderSvg() const;
};

} // namespace cchart

#endif // C_CHART_H
