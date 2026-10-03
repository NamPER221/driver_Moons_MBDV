#pragma once

/**
 * @file config_path.hpp
 * @brief Resolves configuration file paths so the tools work from any directory.
 *
 * The generated DCF references its slave binary with a path relative to the DCF's own
 * directory (dcfgen writes `UploadFile=slave_1.bin`), and lely resolves those entries
 * against the DCF path. So only the DCF path itself needs to be located here.
 *
 * Without this, `mbdv_dual_axis_node` would work when launched from the project root but
 * fail at stage S01 with "cannot open DCF file" when launched from build/ - exactly the
 * kind of confusing, environment-dependent failure the staged logging exists to prevent.
 */

#include <string>
#include <vector>

namespace mbdv {

/// Absolute directory of the running executable, or "" when it cannot be determined.
std::string ExecutableDir();

/// Absolute current working directory, or "" when it cannot be determined.
std::string WorkingDirectory();

/// Directory part of @p path (without a trailing slash). "." when there is no slash.
std::string DirectoryOf(const std::string& path);

/**
 * @brief Changes the process working directory to @p dir, remembering the previous one.
 *
 * Needed because lely resolves the `DownloadFile` entries of the DCF (`slave_N.bin`)
 * against the *process* working directory, not against the DCF's own directory - see
 * co_sub_get_download_file() in lely's src/co/obj.c, which returns the raw filename.
 *
 * @param previous if non-null, receives the working directory that was active before.
 * @return true on success, false on failure (errno is left set by chdir()).
 */
bool ChangeWorkingDirectory(const std::string& dir, std::string* previous);

/**
 * @brief Resolves @p path, which may be relative to the working directory.
 *
 * Candidates are tried in order:
 *   1. @p path exactly as given (absolute paths and CWD-relative paths);
 *   2. <project root>/<path>   - the build system bakes the project root in, so a path
 *                               typed relative to the project root always resolves to the
 *                               file the user actually named;
 *   3. <exe dir>/../<path>     - the build/ layout used by this project;
 *   4. <exe dir>/<path>;
 *   5. <exe dir>/../config/<basename>  - last resort only.
 *
 * Candidate 5 matches on the file name alone and can therefore pick a *different*
 * configuration that happens to share the name (e.g. config/master.dcf vs
 * config/single_axis_500k/master.dcf). Candidates 1-4 always keep the sub-directory, so
 * they are tried first and this ambiguity can only arise when the given path really is
 * unavailable.
 *
 * @return the first candidate that exists as a regular file, or "" when none matched.
 */
std::string ResolveConfigPath(const std::string& path);

/// Same as ResolveConfigPath() but never returns an empty string: falls back to @p path
/// itself so the caller can still report the name the user typed.
std::string ResolveConfigPathOr(const std::string& path);

/// All candidates ResolveConfigPath() would try, for logging and for error messages.
std::vector<std::string> ConfigPathCandidates(const std::string& path);

}  // namespace mbdv