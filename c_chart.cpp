#include "c_chart.h"
#include <sstream>
#include <algorithm>

namespace cchart {

CommitChartGenerator::CommitChartGenerator()
    : title("MiniVCS Commit Activity Histogram"),
      totalCommits(0),
      currentStreak(0),
      activeDays(0) {}

void CommitChartGenerator::setTitle(const std::string& t) {
    title = t;
}

void CommitChartGenerator::setStats(int total, int streak, int days) {
    totalCommits = total;
    currentStreak = streak;
    activeDays = days;
}

void CommitChartGenerator::addDailyData(const std::string& date, int count) {
    DailyCommit dc;
    dc.date = date;
    dc.count = count;
    dailyCommits.push_back(dc);
}

void CommitChartGenerator::addContributor(const std::string& author, int count, int percentage) {
    ContributorShare cs;
    cs.author = author;
    cs.count = count;
    cs.percentage = percentage;
    contributors.push_back(cs);
}

void CommitChartGenerator::clear() {
    dailyCommits.clear();
    contributors.clear();
    totalCommits = 0;
    currentStreak = 0;
    activeDays = 0;
}

// 1. Renders the Commit Histogram (for Insights -> Commits)
std::string CommitChartGenerator::renderCommitHistogramSvg() const {
    const int width = 840;
    const int height = 270;
    std::ostringstream oss;

    // 1. SVG Canvas and Background
    oss << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 " << width << " " << height << "\" "
        << "width=\"100%\" height=\"auto\" "
        << "style=\"background-color:#0d1117; border:1px solid #30363d; border-radius:10px; font-family:-apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;\">\n";

    // 2. Gradients & Filters
    oss << "  <defs>\n";
    oss << "    <linearGradient id=\"histogramBarGrad\" x1=\"0%\" y1=\"100%\" x2=\"0%\" y2=\"0%\">\n";
    oss << "      <stop offset=\"0%\" stop-color=\"#238636\"/>\n";
    oss << "      <stop offset=\"100%\" stop-color=\"#39d353\"/>\n";
    oss << "    </linearGradient>\n";
    oss << "  </defs>\n";

    // 3. Header Section (Title & C++ Badge)
    oss << "  <text x=\"24\" y=\"32\" fill=\"#e6edf3\" font-size=\"15px\" font-weight=\"600\">" << title << "</text>\n";
    oss << "  <rect x=\"24\" y=\"42\" width=\"145\" height=\"18\" rx=\"9\" fill=\"#1f6feb\" fill-opacity=\"0.15\" stroke=\"#388bfd\" stroke-width=\"0.8\"/>\n";
    oss << "  <text x=\"32\" y=\"55\" fill=\"#58a6ff\" font-size=\"10px\" font-weight=\"600\">⚡ C++ Chart Library</text>\n";

    // KPI Badges on the right of header
    oss << "  <g transform=\"translate(" << (width - 420) << ", 20)\">\n";
    oss << "    <text x=\"0\" y=\"18\" fill=\"#8b949e\" font-size=\"12px\">Total Commits: <tspan fill=\"#39d353\" font-weight=\"bold\">" << totalCommits << "</tspan></text>\n";
    oss << "    <text x=\"140\" y=\"18\" fill=\"#8b949e\" font-size=\"12px\">Active Streak: <tspan fill=\"#f0883e\" font-weight=\"bold\">" << currentStreak << " days</tspan></text>\n";
    oss << "    <text x=\"270\" y=\"18\" fill=\"#8b949e\" font-size=\"12px\">Active Days: <tspan fill=\"#58a6ff\" font-weight=\"bold\">" << activeDays << "</tspan></text>\n";
    oss << "  </g>\n";

    // Divider Line
    oss << "  <line x1=\"24\" y1=\"72\" x2=\"" << (width - 24) << "\" y2=\"72\" stroke=\"#30363d\" stroke-width=\"1\"/>\n";

    if (dailyCommits.empty()) {
        oss << "  <text x=\"24\" y=\"150\" fill=\"#6e7681\" font-size=\"13px\">No commits recorded yet. Create a commit to see the histogram increase in real time!</text>\n";
    } else {
        // Find max commit count for scaling
        int maxCount = 1;
        for (const auto& d : dailyCommits) {
            if (d.count > maxCount) maxCount = d.count;
        }

        const int maxBarHeight = 110;
        const int baselineY    = 220;
        const int chartStartX  = 46;
        const int barWidth     = 34;
        const int barSpacing   = 52;

        // Gridlines & Y-Axis values
        oss << "  <g stroke=\"#21262d\" stroke-width=\"1\" stroke-dasharray=\"3,3\">\n";
        // Top line
        oss << "    <line x1=\"40\" y1=\"" << (baselineY - maxBarHeight) << "\" x2=\"" << (width - 30) << "\" y2=\"" << (baselineY - maxBarHeight) << "\"/>\n";
        // Mid line
        oss << "    <line x1=\"40\" y1=\"" << (baselineY - maxBarHeight / 2) << "\" x2=\"" << (width - 30) << "\" y2=\"" << (baselineY - maxBarHeight / 2) << "\"/>\n";
        oss << "  </g>\n";

        // Y-axis labels
        oss << "  <text x=\"34\" y=\"" << (baselineY - maxBarHeight + 4) << "\" fill=\"#6e7681\" font-size=\"9px\" text-anchor=\"end\">" << maxCount << "</text>\n";
        oss << "  <text x=\"34\" y=\"" << (baselineY - maxBarHeight / 2 + 4) << "\" fill=\"#6e7681\" font-size=\"9px\" text-anchor=\"end\">" << (maxCount / 2) << "</text>\n";
        oss << "  <text x=\"34\" y=\"" << (baselineY + 4) << "\" fill=\"#6e7681\" font-size=\"9px\" text-anchor=\"end\">0</text>\n";

        // Solid Baseline axis
        oss << "  <line x1=\"40\" y1=\"" << baselineY << "\" x2=\"" << (width - 30) << "\" y2=\"" << baselineY << "\" stroke=\"#30363d\" stroke-width=\"1.5\"/>\n";

        // Histogram Bars
        for (size_t i = 0; i < dailyCommits.size() && i < 15; i++) {
            int cx = chartStartX + (int)i * barSpacing;
            int count = dailyCommits[i].count;

            int barHeight = (count * maxBarHeight) / maxCount;
            if (count > 0 && barHeight < 6) barHeight = 6;
            int barY = baselineY - barHeight;

            // Background slot guide
            oss << "  <rect x=\"" << cx << "\" y=\"" << (baselineY - maxBarHeight) << "\" width=\"" << barWidth
                << "\" height=\"" << maxBarHeight << "\" rx=\"4\" fill=\"#161b22\" fill-opacity=\"0.6\"/>\n";

            // Active commit histogram bar
            if (count > 0) {
                oss << "  <rect x=\"" << cx << "\" y=\"" << barY << "\" width=\"" << barWidth
                    << "\" height=\"" << barHeight << "\" rx=\"4\" fill=\"url(#histogramBarGrad)\">\n";
                oss << "    <title>" << count << " commit(s) on " << dailyCommits[i].date << "</title>\n";
                oss << "  </rect>\n";

                // Numeric count tag on top of bar
                oss << "  <text x=\"" << (cx + barWidth / 2) << "\" y=\"" << (barY - 5)
                    << "\" fill=\"#e6edf3\" font-size=\"11px\" font-weight=\"bold\" text-anchor=\"middle\">"
                    << count << "</text>\n";
            }

            // Date label below baseline (MM-DD)
            std::string label = dailyCommits[i].date;
            if (label.size() >= 10) {
                label = label.substr(5);
            }
            oss << "  <text x=\"" << (cx + barWidth / 2) << "\" y=\"" << (baselineY + 16)
                << "\" fill=\"#8b949e\" font-size=\"10px\" text-anchor=\"middle\">" << label << "</text>\n";
        }
    }

    oss << "</svg>";
    return oss.str();
}

// 2. Renders the Contributor Share Chart (for Insights -> Contributors)
std::string CommitChartGenerator::renderContributorChartSvg() const {
    const int width = 840;
    const int height = 230;
    std::ostringstream oss;

    oss << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 " << width << " " << height << "\" "
        << "width=\"100%\" height=\"auto\" "
        << "style=\"background-color:#0d1117; border:1px solid #30363d; border-radius:10px; font-family:-apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;\">\n";

    // Header
    oss << "  <text x=\"24\" y=\"32\" fill=\"#e6edf3\" font-size=\"15px\" font-weight=\"600\">Contributor Contribution Breakdown</text>\n";
    oss << "  <rect x=\"24\" y=\"42\" width=\"145\" height=\"18\" rx=\"9\" fill=\"#1f6feb\" fill-opacity=\"0.15\" stroke=\"#388bfd\" stroke-width=\"0.8\"/>\n";
    oss << "  <text x=\"32\" y=\"55\" fill=\"#58a6ff\" font-size=\"10px\" font-weight=\"600\">⚡ C++ Chart Library</text>\n";

    oss << "  <line x1=\"24\" y1=\"72\" x2=\"" << (width - 24) << "\" y2=\"72\" stroke=\"#30363d\" stroke-width=\"1\"/>\n";

    if (contributors.empty()) {
        oss << "  <text x=\"24\" y=\"120\" fill=\"#6e7681\" font-size=\"13px\">No contributor data available yet.</text>\n";
    } else {
        int shareStartX = 24;
        const int shareTrackWidth = width - 48;
        int currentX = shareStartX;

        // Proportional progress bar
        oss << "  <rect x=\"" << shareStartX << "\" y=\"95\" width=\"" << shareTrackWidth
            << "\" height=\"16\" rx=\"8\" fill=\"#21262d\"/>\n";

        const char* shareColors[] = {"#2ea043", "#388bfd", "#f0883e", "#a371f7", "#ec6547"};
        for (size_t i = 0; i < contributors.size(); i++) {
            int segWidth = (contributors[i].percentage * shareTrackWidth) / 100;
            if (segWidth < 6 && contributors[i].percentage > 0) segWidth = 6;

            std::string cColor = shareColors[i % 5];
            oss << "  <rect x=\"" << currentX << "\" y=\"95\" width=\"" << segWidth
                << "\" height=\"16\" rx=\"8\" fill=\"" << cColor << "\">\n";
            oss << "    <title>" << contributors[i].author << ": " << contributors[i].count
                << " commits (" << contributors[i].percentage << "%)</title>\n";
            oss << "  </rect>\n";
            currentX += segWidth;
        }

        // Contributor Badges & Cards below bar
        int cardX = 24;
        for (size_t i = 0; i < contributors.size(); i++) {
            std::string cColor = shareColors[i % 5];

            // Card container
            oss << "  <rect x=\"" << cardX << "\" y=\"135\" width=\"240\" height=\"70\" rx=\"8\" fill=\"#161b22\" stroke=\"#30363d\" stroke-width=\"1\"/>\n";
            oss << "  <circle cx=\"" << (cardX + 24) << "\" cy=\"170\" r=\"12\" fill=\"" << cColor << "\"/>\n";
            oss << "  <text x=\"" << (cardX + 24) << "\" y=\"174\" fill=\"#ffffff\" font-size=\"11px\" font-weight=\"bold\" text-anchor=\"middle\">"
                << (char)toupper(contributors[i].author[0]) << "</text>\n";

            oss << "  <text x=\"" << (cardX + 46) << "\" y=\"162\" fill=\"#e6edf3\" font-size=\"13px\" font-weight=\"600\">"
                << contributors[i].author << "</text>\n";
            oss << "  <text x=\"" << (cardX + 46) << "\" y=\"182\" fill=\"#8b949e\" font-size=\"11px\">"
                << contributors[i].count << " commits (<tspan fill=\"" << cColor << "\" font-weight=\"bold\">"
                << contributors[i].percentage << "%</tspan>)</text>\n";

            cardX += 260;
        }
    }

    oss << "</svg>";
    return oss.str();
}

std::string CommitChartGenerator::renderSvg() const {
    return renderCommitHistogramSvg();
}

} // namespace cchart
