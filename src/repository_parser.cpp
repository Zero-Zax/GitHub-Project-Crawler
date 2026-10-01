#include "repository_parser.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<Repository> parse_repositories(const std::string& body) {
    // turn the text into a json object we can look things up in.
    json root = json::parse(body);

    std::vector<Repository> repos;

    // github puts the list of repositories in the "items" array.
    for (const auto& item : root.at("items")) {
        Repository r;
        r.owner = item.at("owner").at("login").get<std::string>();
        r.name = item.at("name").get<std::string>();
        r.stars = item.at("stargazers_count").get<std::uint64_t>();
        r.forks = item.at("forks_count").get<std::uint64_t>();
        r.size = item.at("size").get<std::uint64_t>();
        r.updated_at = item.at("updated_at").get<std::string>();
        r.url = item.at("html_url").get<std::string>();

        // some repos have no description or language (null in the json),
        // so only copy them if they exist. otherwise they stay "".
        if (!item.at("description").is_null()) {
            r.description = item.at("description").get<std::string>();
        }
        if (!item.at("language").is_null()) {
            r.language = item.at("language").get<std::string>();
        }

        repos.push_back(r);
    }

    return repos;
}
