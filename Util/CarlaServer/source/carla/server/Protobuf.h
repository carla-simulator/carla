// CARLA, Copyright (C) 2017 Computer Vision Center (CVC) Project Settings.

#pragma once

#include <google/protobuf/arena.h>
#include <google/protobuf/message_lite.h>

namespace carla {
namespace server {

  /// Wrapper around google's protobuf library. It holds a protobuf arena for a
  /// more efficient memory usage.
  ///
  /// Encode functions return a string encoded as follows
  ///
  ///    [(uint32_t)message size, <google's protobuf encoding>...]
  ///
  class Protobuf {
  public:

    /// Prepends the size of the message to the string. Only for testing
    /// purposes, for protobuf objects use specilized version of "encode"
    /// function.
    static std::string Encode(const std::string &message);

    /// Efficiently retrieve the string message from the protocol buffer
    /// message.
    static std::string Encode(const google::protobuf::MessageLite &message);

    /// Creates a protobuf message using arena allocation.
    template <typename T>
    T *CreateMessage() {
      return google::protobuf::Arena::CreateMessage<T>(&_arena);
    }

  private:

    google::protobuf::Arena _arena;
  };

} // namespace server
} // namespace carla
