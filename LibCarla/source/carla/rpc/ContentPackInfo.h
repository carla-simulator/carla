// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <string>
#include <vector>

namespace carla {
namespace rpc {

  /// Description of a CARLA content pack known to the server (see
  /// UCarlaContentPackManager). A pack is a content-only Unreal plugin cooked
  /// against a base release, wrapped with a carla-pack.json manifest.
  class ContentPackInfo {
  public:

    ContentPackInfo() = default;

    /// Pack name (== plugin name == mount point "/<name>/").
    std::string name;

    /// Pack version from the manifest.
    std::string version;

    /// Base release the pack was cooked against, e.g. "carla-0.10.2-Linux".
    std::string base_release;

    /// Absolute path of the pack directory on the server host.
    std::string path;

    /// True while the pack is mounted in the running server.
    bool mounted = false;

    /// Map names declared by the pack manifest (loadable with load_world).
    std::vector<std::string> maps;

    MSGPACK_DEFINE_ARRAY(name, version, base_release, path, mounted, maps);
  };

} // namespace rpc
} // namespace carla
