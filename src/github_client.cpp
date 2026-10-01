#include "github_client.h"

#include <httplib.h>

#include <cstdlib>
#include <iostream>

std::string GitHubClient::search_repositories(const SearchOptions& options) {
    // connect to github's api server.
    httplib::Client client("https://api.github.com");

    // build the url, for example: /search/repositories?q=stars%3a%3e100&per_page=100
    // encode_uri_component turns characters like ':' and '>' into url-safe codes.
    std::string path = "/search/repositories?q=" +
                       httplib::encode_uri_component(options.query) +
                       "&per_page=" + std::to_string(options.count);

    // headers that github wants on every request.
    httplib::Headers headers{
        {"Accept", "application/vnd.github+json"},
        {"X-GitHub-Api-Version", "2026-03-10"},
        {"User-Agent", "cs3460-github-crawler"}
    };

    // if GITHUB_TOKEN is set, send it so github gives us a higher rate limit.
    const char* token = std::getenv("GITHUB_TOKEN");
    if (token != nullptr) {
        headers.emplace("Authorization", std::string("Bearer ") + token);
    }

    // send the get request.
    auto result = client.Get(path, headers);

    // no response at all (no internet, bad ssl setup, etc).
    if (!result) {
        std::cerr << "Error: could not connect to GitHub\n";
        return "";
    }

    // github answered, but with an error (403 = rate limited, 422 = bad query, ...).
    if (result->status != 200) {
        std::cerr << "Error: GitHub returned status " << result->status << "\n";
        std::cerr << result->body << "\n";
        return "";
    }

    return result->body;
}
