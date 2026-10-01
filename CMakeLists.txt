cmake_minimum_required(VERSION 3.20)
project(github_crawler LANGUAGES CXX)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/third_party/install")

find_package(httplib CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(github_crawler
    src/main.cpp
    src/github_client.cpp
    src/repository_parser.cpp
    src/repository_store.cpp
)
target_compile_features(github_crawler PRIVATE cxx_std_20)
target_compile_options(github_crawler PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(github_crawler PRIVATE
    httplib::httplib
    nlohmann_json::nlohmann_json
)
