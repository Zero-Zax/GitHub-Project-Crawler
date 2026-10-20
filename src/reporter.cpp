#include "reporter.h"
#include "repository_store.h"

#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>

// cuts text down to width characters so long names don't break the table.
static std::string fit(const std::string& text, std::size_t width) {
    if (text.size() <= width) {
        return text;
    }
    return text.substr(0, width - 3) + "...";
}

void print_repository_table(const std::vector<Repository>& rows) {
    std::cout << std::left << std::setw(40) << "OWNER/NAME" << std::right << std::setw(10)
              << "STARS" << std::setw(10) << "FORKS" << "  " << std::left << std::setw(16)
              << "LANGUAGE" << "UPDATED\n";

    for (const Repository& r : rows) {
        // some repos have no language, show a dash instead of a blank.
        std::string language = r.language.empty() ? "-" : r.language;

        std::cout << std::left << std::setw(40) << fit(r.owner + "/" + r.name, 39) << std::right
                  << std::setw(10) << r.stars << std::setw(10) << r.forks << "  " << std::left
                  << std::setw(16) << fit(language, 15) << r.updated_at << "\n";
    }
}

// wraps a csv field in quotes and doubles any quotes inside it,
// so commas and quotes in descriptions don't break the columns.
static std::string csv_field(const std::string& text) {
    std::string out = "\"";
    for (char c : text) {
        if (c == '"') {
            out += "\"\"";
        } else {
            out += c;
        }
    }
    out += "\"";
    return out;
}

bool save_repositories_csv(const std::vector<Repository>& repos, const std::string& path) {
    std::ofstream file(path);
    if (!file) {
        return false;
    }

    // same fields, same order as the json export.
    file << "owner,name,description,stars,forks,language,size,updated_at,url\n";
    for (const Repository& r : repos) {
        file << csv_field(r.owner) << "," << csv_field(r.name) << "," << csv_field(r.description)
             << "," << r.stars << "," << r.forks << "," << csv_field(r.language) << "," << r.size
             << "," << csv_field(r.updated_at) << "," << csv_field(r.url) << "\n";
    }
    return static_cast<bool>(file);
}

bool export_repositories(const std::vector<Repository>& repos, const std::string& path) {
    bool is_csv = path.size() >= 4 && path.compare(path.size() - 4, 4, ".csv") == 0;
    if (is_csv) {
        return save_repositories_csv(repos, path);
    }
    // json uses the same writer milestone 1 used for repositories.json.
    return save_repositories(repos, path);
}
