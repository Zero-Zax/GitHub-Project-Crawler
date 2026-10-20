#pragma once

#include "repository.h"

#include <filesystem>
#include <vector>

// reads the repositories.json file saved by milestone 1 back into repository structs.
// throws an exception if the file can't be opened or the json is broken.
std::vector<Repository> load_repositories(const std::filesystem::path& path);
