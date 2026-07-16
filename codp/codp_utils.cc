#include "codp_commands.hh"

#include <array>
#include <cstdio>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <sys/wait.h>
#endif

namespace {

#ifdef _WIN32

std::wstring widen_argument(std::string_view value) {
  if (value.empty()) {
    return {};
  }

  UINT code_page = CP_UTF8;
  DWORD flags = MB_ERR_INVALID_CHARS;
  int required = MultiByteToWideChar(code_page, flags, value.data(), static_cast<int>(value.size()), nullptr, 0);
  if (required == 0) {
    code_page = CP_ACP;
    flags = 0;
    required = MultiByteToWideChar(code_page, flags, value.data(), static_cast<int>(value.size()), nullptr, 0);
  }
  if (required == 0) {
    throw std::system_error(static_cast<int>(GetLastError()), std::system_category(),
                            "Failed to convert command argument to UTF-16");
  }

  std::wstring result(static_cast<size_t>(required), L'\0');
  if (MultiByteToWideChar(code_page, flags, value.data(), static_cast<int>(value.size()), result.data(), required) ==
      0) {
    throw std::system_error(static_cast<int>(GetLastError()), std::system_category(),
                            "Failed to convert command argument to UTF-16");
  }
  return result;
}

std::wstring quote_windows_argument(const std::wstring &value) {
  if (value.empty()) {
    return L"\"\"";
  }

  if (value.find_first_of(L" \t\n\r\"") == std::wstring::npos) {
    return value;
  }

  std::wstring quoted;
  quoted.reserve(value.size() + 2);
  quoted.push_back(L'"');
  size_t backslashes = 0;
  for (wchar_t ch : value) {
    if (ch == L'\\') {
      ++backslashes;
    } else if (ch == L'"') {
      quoted.append(backslashes * 2 + 1, L'\\');
      quoted.push_back(ch);
      backslashes = 0;
    } else {
      quoted.append(backslashes, L'\\');
      backslashes = 0;
      quoted.push_back(ch);
    }
  }
  quoted.append(backslashes * 2, L'\\');
  quoted.push_back(L'"');
  return quoted;
}

#else

std::string quote_shell_argument(const std::string &arg) {
  if (arg.empty()) {
    return "''";
  }

  constexpr std::string_view safe_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_+-=.,/:@%";
  if (arg.find_first_not_of(safe_chars) == std::string::npos) {
    return arg;
  }

  std::string quoted;
  quoted.reserve(arg.size() + 2);
  quoted.push_back('\'');
  for (char ch : arg) {
    if (ch == '\'') {
      quoted += "'\\''";
    } else {
      quoted.push_back(ch);
    }
  }
  quoted.push_back('\'');
  return quoted;
}

int normalize_process_status(int status) {
  if (status == -1) {
    return -1;
  }
  if (WIFEXITED(status)) {
    return WEXITSTATUS(status);
  }
  return 128;
}

#endif

} // namespace

CommandResult run_command(const std::vector<std::string> &arguments, bool capture_output) {
  if (arguments.empty()) {
    throw std::invalid_argument("Cannot run an empty command");
  }

#ifdef _WIN32
  std::wstring command_line;
  for (const auto &argument : arguments) {
    if (!command_line.empty()) {
      command_line.push_back(L' ');
    }
    command_line += quote_windows_argument(widen_argument(argument));
  }
  std::vector<wchar_t> command_buffer(command_line.begin(), command_line.end());
  command_buffer.push_back(L'\0');

  SECURITY_ATTRIBUTES security_attributes{};
  security_attributes.nLength = sizeof(security_attributes);
  security_attributes.bInheritHandle = TRUE;

  HANDLE output_read = nullptr;
  HANDLE output_write = nullptr;
  if (capture_output) {
    if (!CreatePipe(&output_read, &output_write, &security_attributes, 0)) {
      throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "Failed to create pipe");
    }
    if (!SetHandleInformation(output_read, HANDLE_FLAG_INHERIT, 0)) {
      const DWORD handle_error = GetLastError();
      CloseHandle(output_read);
      CloseHandle(output_write);
      throw std::system_error(static_cast<int>(handle_error), std::system_category(),
                              "Failed to make command output pipe private");
    }
  }

  STARTUPINFOW startup_info{};
  startup_info.cb = sizeof(startup_info);
  HANDLE null_input = INVALID_HANDLE_VALUE;
  if (capture_output) {
    null_input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &security_attributes,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (null_input == INVALID_HANDLE_VALUE) {
      const DWORD input_error = GetLastError();
      CloseHandle(output_read);
      CloseHandle(output_write);
      throw std::system_error(static_cast<int>(input_error), std::system_category(),
                              "Failed to open NUL for command input");
    }
    startup_info.dwFlags = STARTF_USESTDHANDLES;
    startup_info.hStdInput = null_input;
    startup_info.hStdOutput = output_write;
    startup_info.hStdError = output_write;
  }

  PROCESS_INFORMATION process_info{};
  const BOOL created = CreateProcessW(nullptr, command_buffer.data(), nullptr, nullptr, capture_output ? TRUE : FALSE,
                                      0, nullptr, nullptr, &startup_info, &process_info);
  const DWORD create_error = GetLastError();
  if (null_input != INVALID_HANDLE_VALUE) {
    CloseHandle(null_input);
  }
  if (output_write) {
    CloseHandle(output_write);
  }
  if (!created) {
    if (output_read) {
      CloseHandle(output_read);
    }
    throw std::system_error(static_cast<int>(create_error), std::system_category(),
                            "Failed to start command: " + arguments.front());
  }

  std::string output;
  DWORD read_error = ERROR_SUCCESS;
  if (capture_output) {
    std::array<char, 4096> buffer{};
    while (true) {
      DWORD bytes_read = 0;
      if (!ReadFile(output_read, buffer.data(), static_cast<DWORD>(buffer.size()), &bytes_read, nullptr)) {
        read_error = GetLastError();
        if (read_error == ERROR_BROKEN_PIPE) {
          read_error = ERROR_SUCCESS;
        }
        break;
      }
      if (bytes_read == 0) {
        break;
      }
      output.append(buffer.data(), bytes_read);
    }
    CloseHandle(output_read);
  }

  const DWORD wait_result = WaitForSingleObject(process_info.hProcess, INFINITE);
  const DWORD wait_error = wait_result == WAIT_FAILED ? GetLastError() : ERROR_SUCCESS;
  DWORD exit_code = 0;
  const BOOL got_exit_code = wait_result != WAIT_FAILED && GetExitCodeProcess(process_info.hProcess, &exit_code);
  const DWORD exit_code_error = got_exit_code ? ERROR_SUCCESS : GetLastError();
  CloseHandle(process_info.hThread);
  CloseHandle(process_info.hProcess);
  if (read_error != ERROR_SUCCESS) {
    throw std::system_error(static_cast<int>(read_error), std::system_category(),
                            "Failed to read command output: " + arguments.front());
  }
  if (wait_error != ERROR_SUCCESS) {
    throw std::system_error(static_cast<int>(wait_error), std::system_category(),
                            "Failed to wait for command: " + arguments.front());
  }
  if (!got_exit_code) {
    throw std::system_error(static_cast<int>(exit_code_error), std::system_category(),
                            "Failed to read command exit code: " + arguments.front());
  }
  return {static_cast<int>(exit_code), std::move(output)};
#else
  std::string command;
  for (const auto &argument : arguments) {
    if (!command.empty()) {
      command.push_back(' ');
    }
    command += quote_shell_argument(argument);
  }

  if (!capture_output) {
    return {normalize_process_status(std::system(command.c_str())), {}};
  }

  command += " 2>&1";
  FILE *pipe = ::popen(command.c_str(), "r");
  if (!pipe) {
    throw std::system_error(errno, std::system_category(), "Failed to start command: " + arguments.front());
  }

  std::string output;
  std::array<char, 4096> buffer{};
  while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe)) {
    output += buffer.data();
  }
  return {normalize_process_status(::pclose(pipe)), std::move(output)};
#endif
}

void usage() {
  std::cerr << "codp solve [--project <path>] (default)\n"
               "codp update [--project <path>]\n"
               "codp init [--project <path>] [--uuid <uuid>] <name> <repo_url> <branch>...\n"
               "codp add [--project <path>] <repo_url> <branch>... [--uuid <uuid>]\n"
               "codp rm [--project <path>] <uuid-or-name>"
            << std::endl;
}

fs::path home_dir() {
#ifdef _WIN32
  const char *home = std::getenv("USERPROFILE");
#else
  const char *home = std::getenv("HOME");
#endif
  if (!home) {
    throw std::runtime_error("Cannot determine HOME directory");
  }
  return fs::path(home);
}

// Walk upwards from start until CodProject.yaml found or root reached.
std::optional<fs::path> find_project_dir(fs::path start) {
  start = fs::absolute(start);
  for (fs::path p = start; !p.empty(); p = p.parent_path()) {
    if (fs::exists(p / "CodProject.yaml")) {
      return p;
    }
    if (p == p.root_path())
      break;
  }
  return std::nullopt;
}

// Ensure directory exists (mkdir -p style)
void ensure_dir(const fs::path &p) {
  std::error_code ec;
  fs::create_directories(p, ec);
  if (ec) {
    throw std::runtime_error("Failed to create directory: " + p.string() + ": " + ec.message());
  }
}

void ensure_bare_repo(const std::string &url, const fs::path &bare_path) {
  if (fs::exists(bare_path)) {
    // Fetch updates
    auto result = run_command({"git", "-C", bare_path.string(), "fetch", "--all", "--prune"});
    if (result.exit_code != 0) {
      throw std::runtime_error("git fetch failed for " + url);
    }
  } else {
    ensure_dir(bare_path.parent_path());
    auto result = run_command({"git", "clone", "--mirror", url, bare_path.string()});
    if (result.exit_code != 0) {
      throw std::runtime_error("git clone failed for " + url);
    }
  }
}

bool is_remote_repo_url(std::string_view url) {
  return url.starts_with("http://") || url.starts_with("https://") || url.starts_with("ssh://") ||
         url.starts_with("git@") || url.starts_with("ssh:");
}

// Validate that branches are provided
void validate_branches(const regional_fifo<regional_str> &branches, const std::string &context) {
  if (branches.empty()) {
    throw std::runtime_error(context + ": at least one branch must be specified");
  }
}

// Find dependency by UUID or name
const CodDep *find_dependency(const CodProject *project, const std::string &identifier) {
  for (const CodDep &dep : project->deps()) {
    if (dep.uuid().to_string() == identifier) {
      return &dep;
    }
    if (std::string_view(dep.name()) == identifier) {
      return &dep;
    }
  }
  return nullptr;
}

// Test function with known location for address capture
void errThrowingFunction() { //
  throw shilos::yaml::TypeError("Test error from errThrowingFunction");
}

// Dump debug info for the test function
void dumpTestDebugInfo(std::ostream &os) {
  void *func_addr = (void *)errThrowingFunction;
  os << "Obtained address of errThrowingFunction: " << func_addr << std::endl;
  shilos::dumpDebugInfo(func_addr, os);
  os << std::endl;

  os << "Test errThrowingFunction() call...\n";
  errThrowingFunction();
}
