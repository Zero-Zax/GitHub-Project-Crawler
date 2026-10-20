#include "data_loader.h"
#include "github_client.h"
#include "reporter.h"
#include "repository_analyzer.h"
#include "repository_parser.h"
#include "repository_store.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

// usage:
//   milestone 1 (crawl github, save repositories.json):
//     ./github_crawler [query] [count]
//     ./github_crawler                      -> searches "stars:>100", 100 repos
//     ./github_crawler "stars:>1000"        -> custom search, 100 repos
//     ./github_crawler "language:rust" 25   -> custom search, 25 repos
//
//   milestone 2 (analyze the saved repositories.json, no network):
//     ./github_crawler analyze [--input FILE] [--min-stars N] [--language NAME]
//                              [--sort stars|forks|updated] [--top N] [--output FILE]
//     ./github_crawler analyze --min-stars 1000 --top 5
//     ./github_crawler analyze --language Python --sort forks --output selected.csv

// github -> parse -> print -> save. this is milestone 1.
static int run_crawl(int argc, char* argv[]) {
    // default search settings.
    SearchOptions options;
    options.query = "stars:>100";
    options.count = 100;

    // use the command-line arguments if they were given.
    if (argc > 1) {
        options.query = argv[1];
    }
    if (argc > 2) {
        options.count = std::atoi(argv[2]);
    }

    // github only returns up to 100 results per request.
    if (options.count < 1 || options.count > 100) {
        std::cerr << "Error: count must be between 1 and 100\n";
        return 1;
    }

    std::cout << "Searching GitHub for \"" << options.query << "\" (" << options.count
              << " repositories)\n\n";

    // step 1: ask github for the repositories.
    GitHubClient client;
    std::string body = client.search_repositories(options);
    if (body.empty()) {
        return 1;
    }

    // step 2: turn the json text into repository structs.
    std::vector<Repository> repos;
    try {
        repos = parse_repositories(body);
    } catch (const std::exception& e) {
        std::cerr << "Error: could not read GitHub's response: " << e.what() << "\n";
        return 1;
    }

    // step 3: print each repository.
    for (std::size_t i = 0; i < repos.size(); i++) {
        print_repository(i + 1, repos[i]);
    }

    // step 4: save them to a file for milestone 2.
    if (!save_repositories(repos, "repositories.json")) {
        std::cerr << "Error: could not write repositories.json\n";
        return 1;
    }

    std::cout << "\nSummary: collected " << repos.size() << " repositories, saved to repositories.json\n";
    return 0;
}

// turns text like "1000" into a number. returns false if it isn't a plain whole number.
static bool parse_number(const std::string& text, std::uint64_t& out) {
    if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos) {
        return false;
    }
    try {
        out = std::stoull(text);
    } catch (const std::exception&) {
        return false;  // too big to fit
    }
    return true;
}

// load repositories.json -> filter -> sort -> top n -> print + export. this is milestone 2.
// argv[0] is "analyze", the options start at argv[1].
static int run_analyze(int argc, char* argv[]) {
    AnalysisOptions options;
    std::string input_path = "repositories.json";
    std::string output_path = "selected.json";

    // read the --flag value pairs.
    for (int i = 1; i < argc; i++) {
        std::string flag = argv[i];
        if (i + 1 >= argc) {
            std::cerr << "Error: " << flag << " needs a value\n";
            return 1;
        }
        std::string value = argv[++i];

        if (flag == "--input") {
            input_path = value;
        } else if (flag == "--output") {
            output_path = value;
        } else if (flag == "--language") {
            options.language = value;
        } else if (flag == "--sort") {
            options.sort_key = value;
        } else if (flag == "--min-stars") {
            if (!parse_number(value, options.min_stars)) {
                std::cerr << "Error: --min-stars must be a whole number\n";
                return 1;
            }
        } else if (flag == "--top") {
            std::uint64_t n{};
            if (!parse_number(value, n) || n < 1) {
                std::cerr << "Error: --top must be a whole number of at least 1\n";
                return 1;
            }
            options.top_n = n;
        } else {
            std::cerr << "Error: unknown option " << flag << "\n";
            return 1;
        }
    }

    if (!is_valid_sort_key(options.sort_key)) {
        std::cerr << "Error: --sort must be stars, forks, or updated\n";
        return 1;
    }

    // step 1: load the dataset milestone 1 saved.
    std::vector<Repository> repos;
    try {
        repos = load_repositories(input_path);
    } catch (const std::exception& e) {
        std::cerr << "Error: could not load " << input_path << ": " << e.what() << "\n";
        return 1;
    }

    std::cout << "Dataset: " << input_path << "\n";
    std::cout << "Repositories loaded: " << repos.size() << "\n";
    std::cout << "Filter: stars >= " << options.min_stars << "\n";
    std::cout << "Language: " << (options.language.empty() ? "any" : options.language) << "\n";
    std::cout << "Sort: " << options.sort_key << " (descending)\n";
    std::cout << "Top N: " << options.top_n << "\n\n";

    // step 2: filter -> sort -> top n.
    std::vector<Repository> selected = analyze_repositories(repos, options);

    // step 3: print and export the exact same selected list.
    print_repository_table(selected);
    std::cout << "\nSelected repositories: " << selected.size() << "\n";

    if (!export_repositories(selected, output_path)) {
        std::cerr << "Error: could not write " << output_path << "\n";
        return 1;
    }
    std::cout << "Exported to: " << output_path << "\n";
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "analyze") {
        return run_analyze(argc - 1, argv + 1);
    }
    return run_crawl(argc, argv);
}
