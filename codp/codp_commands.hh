#pragma once

#include "codp.hh"
#include "codp_yaml.hh"

#include "shilos/di.hh"

#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string_view>
#include <unordered_set>

using namespace shilos;
using namespace cod::project;
namespace fs = std::filesystem;

// Shared utility functions
void usage();
fs::path home_dir();
std::optional<fs::path> find_project_dir(fs::path start);
void ensure_dir(const fs::path &p);
void ensure_bare_repo(const std::string &url, const fs::path &bare_path);
bool is_remote_repo_url(std::string_view url);
void validate_branches(const regional_fifo<regional_str> &branches, const std::string &context);
const CodDep *find_dependency(const CodProject *project, const std::string &identifier);

// Debug functions
void errThrowingFunction();
void dumpTestDebugInfo(std::ostream &os);

// Command implementations
int cmd_init(int argc, char **argv, int argi, const fs::path &project_path);
int cmd_add(int argc, char **argv, int argi, const fs::path &project_path);
int cmd_rm(int argc, char **argv, int argi, const fs::path &project_path);
int cmd_solve(int argc, char **argv, int argi, const fs::path &project_path);
int cmd_update(int argc, char **argv, int argi, const fs::path &project_path);
int cmd_debug(int argc, char **argv, int argi, const fs::path &project_path);
