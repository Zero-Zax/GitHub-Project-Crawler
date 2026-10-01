#pragma once

#include "repository.h"

#include <string>
#include <vector>

// turns github's json search response into a list of repository structs.
// throws an exception if the json is broken or missing a field.
std::vector<Repository> parse_repositories(const std::string& body);
