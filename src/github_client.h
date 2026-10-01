#pragma once

#include "repository.h"

#include <string>

// talks to the github rest api.
class GitHubClient {
public:
    // searches github and returns the json text it sends back.
    // returns an empty string if something goes wrong.
    std::string search_repositories(const SearchOptions& options);
};
