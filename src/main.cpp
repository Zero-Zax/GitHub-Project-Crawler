#include "github_client.h"
#include "repository_parser.h"
#include "repository_store.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

// usage: ./github_crawler [query] [count]
//   ./github_crawler                      -> searches "stars:>100", 100 repos
//   ./github_crawler "stars:>1000"        -> custom search, 100 repos
//   ./github_crawler "language:rust" 25   -> custom search, 25 repos
int main(int argc, char* argv[]) {
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
