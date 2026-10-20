#pragma once

#include "repository.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// describes what the user wants out of the analysis.
struct AnalysisOptions {
    std::uint64_t min_stars{};
    std::string language;          // empty means any language
    std::string sort_key{"stars"};  // "stars", "forks", or "updated"
    std::size_t top_n{10};
};

// true if the repository passes the star and language filters.
bool matches_filters(const Repository& r, const AnalysisOptions& options);

// returns only the repositories that pass the filters.
std::vector<Repository> filter_repositories(const std::vector<Repository>& input,
                                            const AnalysisOptions& options);

// comparison functions for std::sort. each sorts biggest/newest first,
// and breaks ties by owner then name so the order is always the same.
bool compare_by_stars(const Repository& a, const Repository& b);
bool compare_by_forks(const Repository& a, const Repository& b);
bool compare_by_updated(const Repository& a, const Repository& b);

// true if sort_key is one of the supported sort keys.
bool is_valid_sort_key(const std::string& sort_key);

// sorts the repositories in place using the comparison function for sort_key.
void sort_repositories(std::vector<Repository>& rows, const std::string& sort_key);

// runs the whole analysis: filter -> sort -> top n. returns the selected repositories.
std::vector<Repository> analyze_repositories(const std::vector<Repository>& input,
                                             const AnalysisOptions& options);
