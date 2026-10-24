module;

#include <string>
#include <filesystem>

export module configuring_paths;

import prog_error;

export namespace config_paths {

std::filesystem::path GetAbsPath(const std::string& resource_paths) {

  std::filesystem::path resources(resource_paths);
  std::filesystem::path abs_path = std::filesystem::weakly_canonical(
                                        std::filesystem::absolute(resources)
                                    );

  if (!std::filesystem::exists(resources)) {
    prog_error::ErrorInfo inf = {prog_error::ErrorCode::kTests,
                                   "Error of finding resource folder",
                                   "GetAbsPath"};
    throw prog_error::ProgError(std::move(inf));
  }

  return abs_path;
}

std::filesystem::path GetAbsRootPath(const std::string& relative_bin_paths) {
  // .../root/Build/tests/end_to_end_tests -->  .../root/Build/tests/
  // .../root/Build/tests/ --> .../root/Build
  // .../root/Build --> .../root/

  std::filesystem::path bin_path = GetAbsPath(relative_bin_paths);
  std::filesystem::path root_path = bin_path.parent_path().parent_path().parent_path();

  return root_path;
}

std::string GetAbsMaterials(const std::string& relative_bin_path) {

  std::filesystem::path root_path = GetAbsRootPath(relative_bin_path);
  std::filesystem::path materials = root_path / "materials";
  std::string materials_str = materials.string();

  return materials_str;
}

std::string GetAbsDatasets(const std::string& relative_bin_path) {

  std::filesystem::path root_path = GetAbsRootPath(relative_bin_path);
  std::filesystem::path datasets = root_path / "research" / "datasets";
  std::string datasets_str = datasets.string();

  return datasets_str;
}

} // namespace config_paths
