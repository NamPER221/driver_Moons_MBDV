#include "mbdv/config_path.hpp"

#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <vector>

namespace mbdv {
namespace {

bool IsRegularFile(const std::string& path) {
  if (path.empty()) return false;
  struct stat st;
  if (stat(path.c_str(), &st) != 0) return false;
  return S_ISREG(st.st_mode);
}

std::string Basename(const std::string& path) {
  const std::size_t slash = path.find_last_of('/');
  return slash == std::string::npos ? path : path.substr(slash + 1);
}

std::string Dirname(const std::string& path) {
  const std::size_t slash = path.find_last_of('/');
  if (slash == std::string::npos) return ".";
  if (slash == 0) return "/";
  return path.substr(0, slash);
}

std::string Join(const std::string& dir, const std::string& leaf) {
  if (dir.empty()) return leaf;
  if (dir.back() == '/') return dir + leaf;
  return dir + "/" + leaf;
}

bool IsAbsolute(const std::string& path) { return !path.empty() && path.front() == '/'; }

}  // namespace

std::string ExecutableDir() {
  std::vector<char> buffer(4096);
  const ssize_t length = ::readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
  if (length <= 0) return "";
  buffer[static_cast<std::size_t>(length)] = '\0';
  return Dirname(std::string(buffer.data()));
}

std::string WorkingDirectory() {
  std::vector<char> buffer(4096);
  if (::getcwd(buffer.data(), buffer.size()) == nullptr) return "";
  return std::string(buffer.data());
}

std::string DirectoryOf(const std::string& path) {
  if (path.empty()) return ".";
  std::string trimmed = path;
  while (trimmed.size() > 1 && trimmed.back() == '/') {
    trimmed.pop_back();
  }
  return Dirname(trimmed);
}

bool ChangeWorkingDirectory(const std::string& dir, std::string* previous) {
  const std::string before = WorkingDirectory();
  if (!dir.empty() && ::chdir(dir.c_str()) == 0) {
    if (previous) *previous = before;
    return true;
  }
  return false;
}

std::vector<std::string> ConfigPathCandidates(const std::string& path) {
  std::vector<std::string> candidates;
  if (path.empty()) return candidates;

  // 1. exactly as given (absolute, or relative to the working directory)
  candidates.push_back(path);

  const std::string exe_dir = ExecutableDir();
  const std::string exe_parent = exe_dir.empty() ? std::string() : Join(exe_dir, "..");

  // 2. relative to the project root baked in at build time - this keeps the
  //    sub-directory the user typed (config/<other>/... stays distinct).
#ifdef MBDV_PROJECT_DIR
  if (!IsAbsolute(path)) {
    candidates.push_back(Join(MBDV_PROJECT_DIR, path));
  }
#endif

  // 3. relative to the executable's parent (the build/ layout)
  if (!exe_parent.empty() && !IsAbsolute(path)) {
    candidates.push_back(Join(exe_parent, path));
  }

  // 4. relative to the executable itself
  if (!exe_dir.empty() && !IsAbsolute(path)) {
    candidates.push_back(Join(exe_dir, path));
  }

  // 5. last resort: config/<basename> next to the project root. Matches on the file
  //    name only, so it is tried after every candidate that keeps the sub-directory.
  if (!exe_parent.empty()) {
    candidates.push_back(Join(exe_parent, Join("config", Basename(path))));
  }

  return candidates;
}

std::string ResolveConfigPath(const std::string& path) {
  for (const std::string& candidate : ConfigPathCandidates(path)) {
    if (!IsRegularFile(candidate)) continue;
    // Always hand back an absolute path. The caller changes the working directory to the
    // DCF's directory so that the DCF's relative slave_N.bin references resolve, and a
    // relative DCF path would stop resolving the moment that chdir happens.
    if (IsAbsolute(candidate)) return candidate;
    const std::string cwd = WorkingDirectory();
    return cwd.empty() ? candidate : Join(cwd, candidate);
  }
  return "";
}

std::string ResolveConfigPathOr(const std::string& path) {
  const std::string resolved = ResolveConfigPath(path);
  return resolved.empty() ? path : resolved;
}

}  // namespace mbdv