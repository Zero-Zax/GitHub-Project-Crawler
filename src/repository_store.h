#pragma once

#include "repository.h"

#include <cstddef>
#include <string>
#include <vector>

// prints one repository to the screen. number is its spot in the list (1, 2, 3, ...).
void print_repository(std::size_t number, const Repository& repo);

// saves all the repositories to a json file. returns false if the file couldn't be written.
bool save_repositories(const std::vector<Repository>& repos, const std::string& path);
