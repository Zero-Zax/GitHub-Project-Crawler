#include "data_loader.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

std::vector<Repository> load_repositories(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("cannot open repository dataset " + path.string());
    }

    // read the whole file as json. the file should be one big array.
    json root;
    input >> root;
    if (!root.is_array()) {
        throw std::runtime_error("repository dataset must be a json array");
    }

    // rebuild one repository for each object, using the same field names save_repositories wrote.
    std::vector<Repository> repos;
    for (const auto& item : root) {
        Repository r;
        r.owner = item.at("owner").get<std::string>();
        r.name = item.at("name").get<std::string>();
        r.description = item.at("description").get<std::string>();
        r.stars = item.at("stars").get<std::uint64_t>();
        r.forks = item.at("forks").get<std::uint64_t>();
        r.language = item.at("language").get<std::string>();
        r.size = item.at("size").get<std::uint64_t>();
        r.updated_at = item.at("updated_at").get<std::string>();
        r.url = item.at("url").get<std::string>();
        repos.push_back(r);
    }

    return repos;
}
