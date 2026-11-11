# GitHub Project Crawler — System Design

CS 3460 Modern C++ · Zack Kingston · Nov 10, 2026

## Overview

The GitHub Project Crawler collects repository metadata from the GitHub search API, saves it as a local JSON dataset, and analyzes that dataset offline. A run either crawls (query GitHub, save `repositories.json`) or analyzes (load the file, filter, sort, keep the top N, print and export the result).

Milestones 1 and 2 already work as free functions grouped by file. This design organizes them into a small set of classes with one responsibility each. Data sources, analysis and output can then be explained, replaced and tested separately. It is a design proposal: the class refactor, the tests and their measured results will be added in the final milestone.

## 1. Requirements

The system collects one dataset, then answers analysis requests over it without the network. Each requirement states observable behavior that a test can check.

| ID | Requirement |
| --- | --- |
| R1 | Query the GitHub search API for 1 to 100 repositories matching a search string, sending `GITHUB_TOKEN` when it is set. |
| R2 | Save the collected repositories to `repositories.json` as a JSON array with nine fields per repository: owner, name, description, stars, forks, language, size, updated_at, url. |
| R3 | Load `repositories.json` back into `std::vector<Repository>`, report how many were loaded, and reject a missing file or a non-array document. |
| R4 | Keep only repositories with at least a minimum number of stars and, when one is given, a matching language. |
| R5 | Sort by stars, forks, or update time, descending, breaking ties by owner then name; then keep the first N, or all of them if fewer than N remain. |
| R6 | Print the selected repositories as a table and export exactly the same rows to JSON or CSV, chosen by the output file's extension. |

The example below shows one analysis request and its output, taken from the saved 100-repository dataset.

| Example request | Expected output |
| --- | --- |
| `./github_crawler analyze --min-stars 100000 --sort stars --top 3` | `Repositories loaded: 100`, then three rows: codecrafters-io/build-your-own-x (550858 stars), sindresorhus/awesome (512840), public-apis/public-apis (484749); `Selected repositories: 3`; `Exported to: selected.json` |

## 2. System Design

The system has four main abstractions: RepositorySource, RepositoryAnalyzer, RepositorySink, and CrawlerSystem. RepositorySource hides where repositories come from, either GitHub or a saved file. RepositoryAnalyzer turns a full list into the selected rows. RepositorySink hides where results go: the terminal, a JSON file or a CSV file. CrawlerSystem connects them into one run.

Today `main.cpp` calls the HTTP client, parser, loader, analyzer and printers directly, and picks JSON or CSV with a string check. Behind these four abstractions, each piece can be swapped or tested without touching the others. The `Repository` struct from Milestone 1 stays the only representation of a repository.

### 2.1 Class Structure and Relationships

![Figure 1. Proposed classes and their relationships (simplified)](class_structure.png)

*Figure 1. Proposed classes and their relationships (simplified).*

The system uses inheritance for interchangeable parts. GitHubSource and JsonFileSource implement RepositorySource. TableReporter, ListReporter, JsonExporter and CsvExporter implement RepositorySink. CrawlerSystem holds one source and a list of sinks and uses them only through the base interfaces. Neither the analysis code nor the export code knows whether the data came from the network or from disk.

### 2.2 Responsibilities and Main Interfaces

RepositorySource defines how a run gets its data. GitHubSource wraps the existing `GitHubClient::search_repositories` and `parse_repositories`. JsonFileSource wraps `load_repositories`. Each one returns a complete list or throws, and `describe()` names the source for the run header (for example, `repositories.json`).

```cpp
class RepositorySource {
public:
    virtual std::vector<Repository> load() = 0;  // throws on failure
    virtual std::string describe() const = 0;
    virtual ~RepositorySource() = default;
};
```

RepositoryAnalyzer holds one `AnalysisOptions` value and applies Filter, then Sort, then Top N. It wraps the Milestone 2 functions `filter_repositories`, `sort_repositories` and the top-N resize. It never prints or writes files, so the rows it returns are exactly the rows every sink receives.

```cpp
class RepositoryAnalyzer {
private:
    AnalysisOptions options_;

public:
    explicit RepositoryAnalyzer(AnalysisOptions options);
    std::vector<Repository> run(const std::vector<Repository>& input) const;
    void describe(std::ostream& out) const;  // Filter / Language / Sort / Top N lines
};
```

RepositorySink defines how a run delivers its result. TableReporter prints the Milestone 2 table. ListReporter prints the Milestone 1 numbered list. JsonExporter reuses `save_repositories`, and CsvExporter reuses `save_repositories_csv`. Each sink writes the rows it is given and reports success.

```cpp
class RepositorySink {
public:
    virtual bool write(const std::vector<Repository>& rows) = 0;
    virtual ~RepositorySink() = default;
};
```

CrawlerSystem controls the order of work but contains no network, parsing, sorting or formatting code. A crawl is `GitHubSource` → ListReporter + JsonExporter(`repositories.json`). An analysis is `JsonFileSource` → RepositoryAnalyzer → TableReporter + one exporter. `main()` only reads the command line and builds the right objects.

```cpp
class CrawlerSystem {
private:
    std::unique_ptr<RepositorySource> source_;
    std::vector<std::unique_ptr<RepositorySink>> sinks_;

public:
    explicit CrawlerSystem(std::unique_ptr<RepositorySource> source);
    void addSink(std::unique_ptr<RepositorySink> sink);
    bool crawl();                                     // load -> every sink
    bool analyze(const RepositoryAnalyzer& analyzer); // load -> analyze -> every sink
};
```

### 2.3 Interaction for an Analysis Request

Consider `analyze --min-stars 100000 --top 3` with the default `selected.json` output. Every analysis follows the same five steps in order.

1. **Load.** CrawlerSystem calls `source_->load()`. JsonFileSource reads `repositories.json` and returns 100 `Repository` values. CrawlerSystem prints the source name and that count.
2. **Filter.** `RepositoryAnalyzer::run` keeps repositories with at least 100000 stars. The language is unset, so any language passes.
3. **Sort.** The analyzer sorts with `compare_by_stars`, descending, with ties broken by owner then name.
4. **Top N.** The analyzer resizes the list to 3. If fewer than 3 had matched, it would keep all of them.
5. **Report and export.** CrawlerSystem passes the same three rows to TableReporter, then to JsonExporter, and prints the selected count and output path.

A crawl runs the same steps with GitHubSource and no analyzer. Errors stop the run before any sink writes: a missing file, a non-array document, a bad sort key or a failed HTTP request. Together these interactions cover R1 to R6.

## 3. Test Design

Eighteen tests check R1 to R6: eight normal cases (T1–T8) and ten error or edge cases (T9–T18). Each test fixes its input in advance, so its expected result is an exact value, not just "the program runs". The tests use saved files, small hand-built datasets and in-memory test objects, so none of them needs the network. Actual results will be recorded in the final milestone.

### 3.1 Normal Cases

| Test | Target | Input / Setup | Expected Result |
| --- | --- | --- | --- |
| T1 | R1 · `parse_repositories` (GitHubSource) | A saved GitHub search response with 3 items. | 3 `Repository` values. Owner comes from `owner.login`, stars from `stargazers_count`, url from `html_url`. |
| T2 | R2 · JsonExporter | Write 2 known repositories to a temporary file. | A JSON array of 2 objects, each with exactly the nine required fields and the values written. |
| T3 | R3 · JsonFileSource | Load the saved `repositories.json`. | 100 values; `Repositories loaded: 100` is printed; the first is codecrafters-io/build-your-own-x. |
| T4 | R4 · filter | Run `analyze --min-stars 100000`, then `analyze --language Python --top 100`. | Every row has at least 100000 stars. The second run shows only `Python` rows: all 25 in the dataset. |
| T5 | R5 · sort | Run `--sort stars`, `--sort forks` and `--sort updated`, each with `--top 100`. | The sorted column never increases from one row to the next. |
| T6 | R5 · top N | Run `analyze --top 3`. | Exactly 3 rows: the 3 repositories with the most stars. |
| T7 | R6 · CsvExporter, JsonExporter | Run `--top 5 --output selected.csv`, then `--top 5 --output selected.json`. | The CSV has a header and 5 rows; the JSON has 5 objects. Both list the same repositories in the same order as the printed table. |
| T8 | R3–R6 · CrawlerSystem, end to end | A test source holding 4 repositories, a filter that keeps 2, a capturing test sink and a JsonExporter. | The source is loaded once. The test sink and the exported file both receive the same 2 rows in the same order. |

### 3.2 Error and Edge Cases

| Test | Target | Input / Setup | Expected Result |
| --- | --- | --- | --- |
| T9 | R1 · `parse_repositories` | A response item whose `description` and `language` are `null`. | Both fields become `""`; no exception. |
| T10 | R1 · crawl command | A count of `0`, `101` or `abc`; separately, a response body that is invalid JSON or has no `items` array. | Each count is rejected with `count must be between 1 and 100` before any request is sent. The bad body prints `could not read GitHub's response`. Both exit with code 1, and `repositories.json` is left unchanged. |
| T11 | R3 · JsonFileSource | `--input missing.json`, a path that does not exist. | `cannot open repository dataset` is printed, with exit code 1. No table is printed and no export file is written. |
| T12 | R3 · JsonFileSource | Three bad files: one holding `{}`, one whose object has no `stars` field, and one holding invalid JSON text. | The first fails with `repository dataset must be a json array`. The other two also fail to load. All three exit with code 1 and write no export file. |
| T13 | R3, R6 · empty dataset | A file containing `[]`. | `Repositories loaded: 0`, then the table header only and `Selected repositories: 0`. The export file contains `[]`. |
| T14 | R4 · filter, no matches | `--language python` (lower case), then `--min-stars 10000000`. | Both select 0 rows without an error, since the language match is case-sensitive and no repository has 10 million stars. The export is `[]`. |
| T15 | R5 · ties | A 4-row dataset where b/x, a/z and a/y tie on stars, forks and update time. Sort by each key, twice. | The tied rows always come out a/y, a/z, b/x, for every key and on both runs. |
| T16 | R5 · top N boundaries | `--language Rust --top 50` (6 Rust repositories), then `--top 100` (exactly 100 loaded). | All 6 Rust rows with no error, then all 100 rows. |
| T17 | CLI · option checks | Each separately: `--sort size`, `--top 0`, `--top abc`, `--min-stars -5`, `--top` with no value, `--color red`. | Each prints its own error: `--sort must be stars, forks, or updated`, `--top must be a whole number of at least 1`, `--min-stars must be a whole number`, `needs a value` or `unknown option`. Each exits with code 1 before loading the dataset. |
| T18 | R6 · CsvExporter | Export 1 repository whose description is `has, comma "q"`. | The CSV field is `"has, comma ""q"""`. A CSV reader finds 9 columns and gets the original description back. |

### 3.3 Coverage of the Requirements

| Requirement | Normal cases | Error / edge cases |
| --- | --- | --- |
| R1 — Collect from GitHub | T1 | T9, T10 |
| R2 — Save dataset | T2 | T7 (exported JSON reloads) |
| R3 — Load dataset | T3, T8 | T11, T12, T13 |
| R4 — Filter | T4, T8 | T14 |
| R5 — Sort and top N | T5, T6 | T15, T16 |
| R6 — Report and export | T7, T8 | T13, T14, T18 |
| Command-line input | — | T17 |

Every requirement has at least one normal case showing it works and one error or edge case showing what happens at its limits. Each component is tested on its own, and T8 checks that CrawlerSystem connects them. If T8 printed the right rows but exported different ones, the fault would be in how CrawlerSystem hands rows to the sinks, not in the analyzer. For each test, the final report will record the actual result and a pass or fail status, and will explain any failure.

## Optional Bonus — Strategy Pattern for Sort Order

The proposed pattern is a Strategy that makes each sort order its own object. RepositoryAnalyzer then sorts through one interface instead of branching on a string.

**The problem.** Today the sort key appears in three places. `is_valid_sort_key` lists the valid names, `sort_repositories` has an if/else chain that picks a comparison function, and the usage text lists the names again. A new order, such as by size, means editing all three and keeping them in step. A typo in one place causes no compile error: the key is accepted, and then the list is silently left unsorted.

**The classes.** RepositoryOrder is the strategy interface. ByStars, ByForks and ByUpdated each implement it and wrap the existing named comparison function, so the ordering and tie-break rules stay as they are. A small `make_order(key)` function returns the matching strategy, or null for an unknown key. That function is the one place that knows the names.

```cpp
class RepositoryOrder {
public:
    virtual bool before(const Repository& a, const Repository& b) const = 0;
    virtual std::string name() const = 0;  // "stars", "forks", "updated"
    virtual ~RepositoryOrder() = default;
};

class ByStars : public RepositoryOrder {
public:
    bool before(const Repository& a, const Repository& b) const override {
        return compare_by_stars(a, b);
    }
    std::string name() const override { return "stars"; }
};

// adapts a strategy to the comparator std::sort expects, without a lambda.
struct OrderComparator {
    const RepositoryOrder& order;
    bool operator()(const Repository& a, const Repository& b) const {
        return order.before(a, b);
    }
};

std::unique_ptr<RepositoryOrder> make_order(const std::string& key);
```

**The difference from the basic design.** In the basic design, RepositoryAnalyzer holds the sort key as a string and branches on it every run. In the Strategy design, `main()` calls `make_order` once while reading the command line. An unknown key is rejected there, and the analyzer receives a `std::unique_ptr<RepositoryOrder>`. The analyzer then calls `std::sort(rows.begin(), rows.end(), OrderComparator{*order_})` and no longer knows which orders exist. Adding an order becomes one new class and one line in `make_order`, and each strategy can be unit-tested on two repositories without running the analyzer.
