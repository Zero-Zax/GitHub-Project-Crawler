#pragma once

#include "repository.h"

#include <string>
#include <vector>

// prints the selected repositories as a table with one row per repository.
void print_repository_table(const std::vector<Repository>& rows);

// saves the repositories to a csv file. returns false if the file couldn't be written.
bool save_repositories_csv(const std::vector<Repository>& repos, const std::string& path);

// saves the repositories as csv if path ends in ".csv", otherwise as json.
// returns false if the file couldn't be written.
bool export_repositories(const std::vector<Repository>& repos, const std::string& path);
