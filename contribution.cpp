#include "contribution.h"
#include "c_chart.h"
#include <ctime>
#include <sstream>
#include <map>

ContributionSummary calculateContributions(CommitNode* head, const string& authorFilter) {
    ContributionSummary summary;
    summary.totalCommits = 0;
    summary.activeDays = 0;
    summary.currentStreak = 0;
    summary.longestStreak = 0;
    summary.activeContributorsCount = 0;

    map<string, int> dateCounts;
    map<string, int> authorCounts;

    // 1. Traverse commit linked list
    CommitNode* curr = head;
    while (curr != nullptr) {
        if (authorFilter.empty() || curr->author == authorFilter) {
            summary.totalCommits++;
            string d = (curr->timestamp.size() >= 10) ? curr->timestamp.substr(0, 10) : "2026-10-17";
            dateCounts[d]++;
            authorCounts[curr->author]++;
        }
        curr = curr->next;
    }

    // 2. Generate sequence for the last 14 days up to today
    time_t now = time(nullptr);
    const int numDays = 14;
    int tempStreak = 0;

    for (int i = numDays - 1; i >= 0; i--) {
        time_t t = now - (i * 86400);
        struct tm* tmInfo = localtime(&t);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d", tmInfo);
        string dt(buf);

        int cnt = (dateCounts.find(dt) != dateCounts.end()) ? dateCounts[dt] : 0;

        DayActivity da;
        da.date = dt;
        da.count = cnt;
        summary.days.push_back(da);

        if (cnt > 0) {
            summary.activeDays++;
            tempStreak++;
            if (tempStreak > summary.longestStreak) {
                summary.longestStreak = tempStreak;
            }
        } else {
            tempStreak = 0;
        }
    }

    // 3. Compute current active streak ending at today (or yesterday)
    int curStreak = 0;
    for (int i = (int)summary.days.size() - 1; i >= 0; i--) {
        if (summary.days[i].count > 0) {
            curStreak++;
        } else {
            if (i == (int)summary.days.size() - 1) continue; // Allow today to start at 0
            break;
        }
    }
    summary.currentStreak = curStreak;

    // 4. Compute contributor share percentages
    for (const auto& pair : authorCounts) {
        AuthorStats as;
        as.author = pair.first;
        as.commitCount = pair.second;
        as.percentage = (summary.totalCommits > 0) ? (pair.second * 100) / summary.totalCommits : 0;
        summary.authorBreakdown.push_back(as);
    }
    summary.activeContributorsCount = (int)summary.authorBreakdown.size();

    return summary;
}

// 1. Commit Activity Histogram
string generateContributionSvg(const ContributionSummary& summary) {
    cchart::CommitChartGenerator gen;
    gen.setStats(summary.totalCommits, summary.currentStreak, summary.activeDays);

    for (const auto& day : summary.days) {
        gen.addDailyData(day.date, day.count);
    }

    return gen.renderCommitHistogramSvg();
}

// 2. Contributor Contribution Share
string generateContributorSvg(const ContributionSummary& summary) {
    cchart::CommitChartGenerator gen;
    gen.setStats(summary.totalCommits, summary.currentStreak, summary.activeDays);

    for (const auto& auth : summary.authorBreakdown) {
        gen.addContributor(auth.author, auth.commitCount, auth.percentage);
    }

    return gen.renderContributorChartSvg();
}

static string escapeForJson(const string& s) {
    string out = "";
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else out += c;
    }
    return out;
}

string contributionsToJson(const ContributionSummary& summary) {
    ostringstream oss;
    oss << "{\n";
    oss << "  \"totalCommits\": " << summary.totalCommits << ",\n";
    oss << "  \"activeDays\": " << summary.activeDays << ",\n";
    oss << "  \"currentStreak\": " << summary.currentStreak << ",\n";
    oss << "  \"longestStreak\": " << summary.longestStreak << ",\n";
    oss << "  \"activeContributorsCount\": " << summary.activeContributorsCount << ",\n";

    oss << "  \"authorBreakdown\": [\n";
    for (size_t i = 0; i < summary.authorBreakdown.size(); i++) {
        oss << "    {\"author\": \"" << summary.authorBreakdown[i].author
            << "\", \"commitCount\": " << summary.authorBreakdown[i].commitCount
            << ", \"percentage\": " << summary.authorBreakdown[i].percentage << "}";
        if (i + 1 < summary.authorBreakdown.size()) oss << ",";
        oss << "\n";
    }
    oss << "  ],\n";

    oss << "  \"days\": [\n";
    for (size_t i = 0; i < summary.days.size(); i++) {
        oss << "    {\"date\": \"" << summary.days[i].date << "\", "
            << "\"count\": " << summary.days[i].count << "}";
        if (i + 1 < summary.days.size()) oss << ",";
        oss << "\n";
    }
    oss << "  ],\n";

    // Embed C++ generated Commit Histogram SVG
    string chartSvg = generateContributionSvg(summary);
    oss << "  \"chartSvg\": \"" << escapeForJson(chartSvg) << "\",\n";

    // Embed C++ generated Contributor Share SVG
    string contribSvg = generateContributorSvg(summary);
    oss << "  \"contributorSvg\": \"" << escapeForJson(contribSvg) << "\"\n";

    oss << "}";
    return oss.str();
}
