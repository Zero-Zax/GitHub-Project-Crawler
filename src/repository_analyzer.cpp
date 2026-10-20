#include "repository_analyzer.h"

#include <algorithm>

bool matches_filters(const Repository& r, const AnalysisOptions& options) {
    if (r.stars < options.min_stars) {
        return false;
    }
    if (!options.language.empty() && r.language != options.language) {
        return false;
    }
    return true;
}

std::vector<Repository> filter_repositories(const std::vector<Repository>& input,
                                            const AnalysisOptions& options) {
    std::vector<Repository> result;
    for (const Repository& r : input) {
        if (matches_filters(r, options)) {
            result.push_back(r);
        }
    }
    return result;
}

bool compare_by_stars(const Repository& a, const Repository& b) {
    if (a.stars != b.stars) {
        return a.stars > b.stars;
    }
    if (a.owner != b.owner) {
        return a.owner < b.owner;
    }
    return a.name < b.name;
}

bool compare_by_forks(const Repository& a, const Repository& b) {
    if (a.forks != b.forks) {
        return a.forks > b.forks;
    }
    if (a.owner != b.owner) {
        return a.owner < b.owner;
    }
    return a.name < b.name;
}

bool compare_by_updated(const Repository& a, const Repository& b) {
    // github timestamps all look like 2026-09-30T21:02:04Z, so comparing
    // them as strings gives the same order as comparing them as times.
    if (a.updated_at != b.updated_at) {
        return a.updated_at > b.updated_at;
    }
    if (a.owner != b.owner) {
        return a.owner < b.owner;
    }
    return a.name < b.name;
}

bool is_valid_sort_key(const std::string& sort_key) {
    return sort_key == "stars" || sort_key == "forks" || sort_key == "updated";
}

void sort_repositories(std::vector<Repository>& rows, const std::string& sort_key) {
    if (sort_key == "stars") {
        std::sort(rows.begin(), rows.end(), compare_by_stars);
    } else if (sort_key == "forks") {
        std::sort(rows.begin(), rows.end(), compare_by_forks);
    } else if (sort_key == "updated") {
        std::sort(rows.begin(), rows.end(), compare_by_updated);
    }
}

std::vector<Repository> analyze_repositories(const std::vector<Repository>& input,
                                             const AnalysisOptions& options) {
    std::vector<Repository> rows = filter_repositories(input, options);
    sort_repositories(rows, options.sort_key);

    // keep only the first n. if fewer than n are left, keep them all.
    if (rows.size() > options.top_n) {
        rows.resize(options.top_n);
    }
    return rows;
}
