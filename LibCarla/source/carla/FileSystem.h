// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

/// @file FileSystem.h
/// @brief File system utilities for CARLA
///
/// Provides static functions for file system operations with automatic
/// directory creation and path validation.

#pragma once

#include <string>
#include <vector>

namespace carla {

  /// @brief Static functions for accessing the file system.
  ///
  /// This class provides convenient file system utilities including path
  /// validation, directory creation, and folder listing. All functions are
  /// static and stateless.
  ///
  /// @warning Using this file requires linking against boost_filesystem.
  ///
  /// @example
  /// ```cpp
  /// std::string path = "output/data.txt";
  /// FileSystem::ValidateFilePath(path, ".txt");
  /// auto files = FileSystem::ListFolder("./data", "*.txt");
  /// ```
  class FileSystem {
  public:
    /// @brief Validate and prepare a file path for writing.
    ///
    /// This function ensures the file path is ready for writing by:
    /// 1. Creating all missing parent directories
    /// 2. Appending the default extension if missing
    ///
    /// @param[in,out] filepath Path to validate and modify.
    ///                         Modified in-place to include extension.
    /// @param[in] default_extension Default extension to append (e.g., ".txt").
    ///                              Optional, defaults to empty string.
    ///
    /// @throws std::runtime_error If directory creation fails.
    ///
    /// @example
    /// ```cpp
    /// std::string path = "output/data";
    /// FileSystem::ValidateFilePath(path, ".txt");
    /// // path is now "output/data.txt" with directories created
    /// ```
    static void ValidateFilePath(
        std::string &filepath,
        const std::string &default_extension = "");

    /// @brief List regular files in a folder matching a pattern.
    ///
    /// Lists all regular files (not directories, not recursive) in the
    /// specified folder that match the given wildcard pattern.
    ///
    /// @param[in] folder_path Path to the folder to list.
    /// @param[in] wildcard_pattern Wildcard pattern for filtering (e.g., "*.txt").
    ///
    /// @return Vector of file paths (full paths, not just names).
    ///
    /// @throws std::invalid_argument If folder does not exist.
    ///
    /// @todo Add permission checks before listing.
    ///
    /// @example
    /// ```cpp
    /// auto txt_files = FileSystem::ListFolder("./data", "*.txt");
    /// for (const auto& file : txt_files) {
    ///   std::cout << file << std::endl;
    /// }
    /// ```
    static std::vector<std::string> ListFolder(
        const std::string &folder_path,
        const std::string &wildcard_pattern);
  };

} // namespace carla
