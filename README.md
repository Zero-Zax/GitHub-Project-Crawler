# GitHub Project Crawler

## Selected Project Topic

**GitHub Project Crawler**

## Team

| Role | Full Name | A-Number | USU Email |
|------|-----------|----------|-----------|
| **Team Leader** | Zack Kingston | A02407688 | A02407688@aggies.usu.edu |


## Requirements

- C++20 (or later)
- Clang / Clang++
- CMake 3.20 or later
- Git
- OpenSSL development headers (needed for HTTPS requests to GitHub)
- An internet connection (Milestone 1 crawl only; Milestone 2 analysis runs offline)
- Command-line build and execution via `./build.sh`

Libraries:

- [cpp-httplib](https://github.com/yhirose/cpp-httplib) (built with OpenSSL), for sending requests to the GitHub REST API
- [nlohmann/json](https://github.com/nlohmann/json), for reading and writing JSON

You don't need to install these yourself. `./build.sh` installs them automatically.

## Build Instructions

From a clean checkout of this repository:

```bash
./build.sh
```


## Run Instructions

### Milestone 1: crawl GitHub

After a successful build:

```bash
./build/github_crawler [query] [count]
```

| Example | Result |
|---|---|
| `./build/github_crawler` | searches `stars:>100`, 100 repositories |
| `./build/github_crawler "stars:>1000"` | custom query, 100 repositories |
| `./build/github_crawler "language:rust" 25` | custom query, 25 repositories |

`count` must be between 1 and 100. Set `GITHUB_TOKEN` in the environment for a higher rate limit.

Expected output (Milestone 1):

```
Searching GitHub for "stars:>100" (100 repositories)

[1] codecrafters-io/build-your-own-x
    Description: Master programming by recreating your favorite technologies from scratch.
    Stars:       550887
    Forks:       51728
    Language:    Markdown
    Size:        1086 KB
    Updated:     2026-10-01T01:31:57Z
    URL:         https://github.com/codecrafters-io/build-your-own-x
...

Summary: collected 100 repositories, saved to repositories.json
```

### Milestone 2: analyze the saved dataset

Milestone 2 loads the `repositories.json` file written by Milestone 1 back into `Repository` values, then filters, sorts, and keeps the top N. It prints the selected repositories and exports exactly the same selection to JSON or CSV. It does not contact GitHub.

```bash
./build/github_crawler analyze [options]
```

| Option | Default | Meaning |
|---|---|---|
| `--input FILE` | `repositories.json` | dataset written by Milestone 1 |
| `--min-stars N` | `0` | keep repositories with at least N stars |
| `--language NAME` | any | keep only this language (case-sensitive, e.g. `Python`) |
| `--sort KEY` | `stars` | `stars`, `forks`, or `updated`, always descending |
| `--top N` | `10` | keep the first N after filtering and sorting |
| `--output FILE` | `selected.json` | export file; a `.csv` name writes CSV, anything else writes JSON |

The steps always run in the order **Load → Filter → Sort → Top N**. Ties are broken by owner, then name, so the output order never changes between runs. If fewer than N repositories match, all of them are shown.

Examples:

```bash
./build/github_crawler analyze --min-stars 1000 --top 5
./build/github_crawler analyze --language Python --sort forks --output selected.csv
./build/github_crawler analyze --sort updated --top 20
```

Expected output (Milestone 2, `analyze --min-stars 1000 --top 5`):

```
Dataset: repositories.json
Repositories loaded: 100
Filter: stars >= 1000
Language: any
Sort: stars (descending)
Top N: 5

OWNER/NAME                                   STARS     FORKS  LANGUAGE        UPDATED
codecrafters-io/build-your-own-x            550858     51727  Markdown        2026-09-30T21:02:04Z
sindresorhus/awesome                        512840     37193  -               2026-09-30T21:02:49Z
public-apis/public-apis                     484749     53554  Python          2026-09-30T21:03:05Z
freeCodeCamp/freeCodeCamp                   456566     47857  TypeScript      2026-09-30T20:15:11Z
EbookFoundation/free-programming-books      398214     66872  Python          2026-09-30T20:39:46Z

Selected repositories: 5
Exported to: selected.json
```

Repositories with no language show `-` in the table and an empty string in the export. Both export formats contain the same nine fields: `owner`, `name`, `description`, `stars`, `forks`, `language`, `size`, `updated_at`, and `url`.

## Source Layout

| File | Responsibility |
|---|---|
| `src/repository.h` | `Repository` value type and `SearchOptions` |
| `src/github_client.*` | M1: HTTPS request to the GitHub search API |
| `src/repository_parser.*` | M1: GitHub JSON response → `std::vector<Repository>` |
| `src/repository_store.*` | M1: print one repository; save repositories to JSON |
| `src/data_loader.*` | M2: `repositories.json` → `std::vector<Repository>` |
| `src/repository_analyzer.*` | M2: `AnalysisOptions`, filtering, named comparison functions, top N |
| `src/reporter.*` | M2: results table; JSON/CSV export of the selected rows |
| `src/main.cpp` | command-line parsing; runs the M1 crawl or the M2 analysis |
