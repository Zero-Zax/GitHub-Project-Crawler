#include "repository_store.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>

using json = nlohmann::json;

void print_repository(std::size_t number, const Repository& repo) {
    std::cout << "[" << number << "] " << repo.owner << "/" << repo.name << "\n";
    std::cout << "    Description: " << repo.description << "\n";
    std::cout << "    Stars:       " << repo.stars << "\n";
    std::cout << "    Forks:       " << repo.forks << "\n";
    std::cout << "    Language:    " << repo.language << "\n";
    std::cout << "    Size:        " << repo.size << " KB\n";
    std::cout << "    Updated:     " << repo.updated_at << "\n";
    std::cout << "    URL:         " << repo.url << "\n";
}

bool save_repositories(const std::vector<Repository>& repos, const std::string& path) {
    // build a json array with one object per repository.
    json out = json::array();
    for (const auto& r : repos) {
        out.push_back({
            {"owner", r.owner},
            {"name", r.name},
            {"description", r.description},
            {"stars", r.stars},
            {"forks", r.forks},
            {"language", r.language},
            {"size", r.size},
            {"updated_at", r.updated_at},
            {"url", r.url}
        });
    }

    std::ofstream file(path);
    if (!file) {
        return false;
    }

    // dump(2) writes the json with 2-space indentation so it's easy to read.
    file << out.dump(2) << "\n";
    return true;
}
