// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include "test.h"

#include <carla/MsgPack.h>
#include <carla/rpc/CommandResponse.h>

#include <vector>

using CommandResponse = carla::rpc::CommandResponse;

TEST(rpc_response_copy, mutable_success_copy_preserves_actor_id) {
  CommandResponse original{42u};
  CommandResponse copy(original);
  ASSERT_FALSE(copy.HasError());
  EXPECT_EQ(copy.Get(), 42u);
}

TEST(rpc_response_copy, mutable_error_copy_preserves_error) {
  CommandResponse original{carla::rpc::ResponseError{"spawn failed"}};
  CommandResponse copy(original);
  ASSERT_TRUE(copy.HasError());
  EXPECT_EQ(copy.GetError().What(), "spawn failed");
}

TEST(rpc_response_copy, copied_batch_retains_values_and_errors_on_wire) {
  std::vector<CommandResponse> original{
      CommandResponse{42u},
      CommandResponse{carla::rpc::ResponseError{"blocked"}}};
  const auto copy = original;
  const auto bytes = carla::MsgPack::Pack(copy);
  const auto unpacked = carla::MsgPack::UnPack<std::vector<CommandResponse>>(
      bytes.data(), bytes.size());
  ASSERT_EQ(unpacked.size(), 2u);
  ASSERT_FALSE(unpacked[0].HasError());
  EXPECT_EQ(unpacked[0].Get(), 42u);
  ASSERT_TRUE(unpacked[1].HasError());
  EXPECT_EQ(unpacked[1].GetError().What(), "blocked");
}

TEST(rpc_response_copy, const_copy_move_and_assignment_preserve_state_controls) {
  const CommandResponse original{42u};
  CommandResponse copy(original);
  CommandResponse moved(std::move(copy));
  CommandResponse assigned{7u};
  assigned = original;
  EXPECT_EQ(original.Get(), 42u);
  EXPECT_EQ(moved.Get(), 42u);
  EXPECT_EQ(assigned.Get(), 42u);
}
