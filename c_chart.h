#ifndef C_CHART_H
#define C_CHART_H

#include <string>
#include <vector>

using namespace std;

namespace cchart {

// Holds daily commit count for the histogram
class DailyCommit {
public:
    string date; // "YYYY-MM-DD" or "MM-DD"
    int count;        // Number of commits

    DailyCommit() : count(0) {}
    DailyCommit(string d, int c) : date(d), count(c) {}
};

// Holds contributor commit share
class ContributorShare {
public:
    string author;
    int count;
    int percentage;

    ContributorShare() : count(0), percentage(0) {}
    ContributorShare(string a, int c, int p) : author(a), count(c), percentage(p) {}
};

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

    void setTitle(const string& t);
    void setStats(int total, int streak, int days);
    void addDailyData(const string& date, int count);
    void addContributor(const string& author, int count, int percentage);
    void clear();

    //  Renders the Commit Histogram (for Insights -> Commits)
    string renderCommitHistogramSvg() const;

    // Renders the Contributor Share Chart (for Insights -> Contributors)
    string renderContributorChartSvg() const;

    // Combined Overview Chart
    string renderSvg() const;
};

} // namespace cchart

#endif // C_CHART_H
