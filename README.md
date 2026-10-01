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
- An internet connection
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

After a successful build:

```bash
./build/github_crawler
```
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
