// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/server/Protobuf.h"

#include "carla/Debug.h"

namespace carla {
namespace server {

  static void PrependByteSize(std::string &string, const uint32_t size) {
    constexpr uint32_t extraSize = sizeof(uint32_t);
    string.reserve(size + extraSize);
    string.assign(reinterpret_cast<const char*>(&size), extraSize);
  }

  std::string Protobuf::Encode(const std::string &message) {
    std::string result;
    PrependByteSize(result, message.size());
    result += message;
    return result;
  }

  std::string Protobuf::Encode(const google::protobuf::MessageLite &message) {
    std::string result;
    DEBUG_ASSERT(message.IsInitialized());
    PrependByteSize(result, message.ByteSize());
    message.AppendToString(&result);
    return result;
  }

} // namespace server
} // namespace carla
