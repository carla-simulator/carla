// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/multigpu/frameGate.h>

#include <limits>

using carla::multigpu::BeginFrameAction;
using carla::multigpu::BeginFrameState;
using carla::multigpu::DecideBeginFrame;
using carla::multigpu::DecidePreTick;
using carla::multigpu::IsConsecutive;
using carla::multigpu::IsUsableDelta;
using carla::multigpu::NeedsResync;
using carla::multigpu::PreTickAction;

namespace {

  BeginFrameState Steady() {
    BeginFrameState state;
    state.connected = true;
    state.has_episode = true;
    return state;
  }

} // namespace

TEST(MultiGpuFrameGate, a_connected_secondary_with_an_episode_waits_for_the_primary) {
  EXPECT_EQ(DecideBeginFrame(Steady()), BeginFrameAction::Wait);
}

TEST(MultiGpuFrameGate, a_disconnected_secondary_runs_on_its_own_clock_whatever_else_holds) {
  BeginFrameState state = Steady();
  state.connected = false;
  EXPECT_EQ(DecideBeginFrame(state), BeginFrameAction::OwnClock);
  state.load_map_pending = true;
  state.travel_pending = true;
  EXPECT_EQ(DecideBeginFrame(state), BeginFrameAction::OwnClock);
}

TEST(MultiGpuFrameGate, never_waits_while_the_primary_waits_for_this_server) {
  // The primary waits for EPISODE_READY, which only the travel sends: waiting
  // here during a load would deadlock both servers until the timeout.
  BeginFrameState travel = Steady();
  travel.travel_pending = true;
  EXPECT_EQ(DecideBeginFrame(travel), BeginFrameAction::DontWait);

  BeginFrameState load_map = Steady();
  load_map.load_map_pending = true;
  EXPECT_EQ(DecideBeginFrame(load_map), BeginFrameAction::DontWait);

  BeginFrameState no_episode = Steady();
  no_episode.has_episode = false;
  EXPECT_EQ(DecideBeginFrame(no_episode), BeginFrameAction::DontWait);

  BeginFrameState exiting = Steady();
  exiting.exit_requested = true;
  EXPECT_EQ(DecideBeginFrame(exiting), BeginFrameAction::DontWait);
}

TEST(MultiGpuFrameGate, a_pending_load_map_takes_precedence_over_a_prepared_frame) {
  EXPECT_EQ(DecidePreTick(true, true, true, true), PreTickAction::OpenLevel);
  // Without an episode there is no world to open the level from yet.
  EXPECT_EQ(DecidePreTick(true, false, false, false), PreTickAction::Skip);
}

TEST(MultiGpuFrameGate, only_a_prepared_frame_on_the_episode_world_is_played) {
  EXPECT_EQ(DecidePreTick(false, true, true, true), PreTickAction::Play);
  EXPECT_EQ(DecidePreTick(false, true, false, true), PreTickAction::Skip);
  EXPECT_EQ(DecidePreTick(false, true, true, false), PreTickAction::Skip);
  EXPECT_EQ(DecidePreTick(false, false, true, true), PreTickAction::Skip);
}

TEST(MultiGpuFrameGate, consecutive_frames) {
  EXPECT_TRUE(IsConsecutive(41u, 42u));
  EXPECT_FALSE(IsConsecutive(41u, 41u));
  EXPECT_FALSE(IsConsecutive(41u, 43u));
  EXPECT_FALSE(IsConsecutive(std::nullopt, 42u));
}

TEST(MultiGpuFrameGate, a_gap_asks_for_a_resync_unless_the_frame_is_one) {
  EXPECT_TRUE(NeedsResync(41u, 43u, false));
  EXPECT_FALSE(NeedsResync(41u, 43u, true));
  EXPECT_FALSE(NeedsResync(41u, 42u, false));
  // The first frame of an episode has nothing to follow.
  EXPECT_FALSE(NeedsResync(std::nullopt, 42u, false));
}

TEST(MultiGpuFrameGate, only_finite_non_negative_deltas_become_the_time_step) {
  EXPECT_TRUE(IsUsableDelta(0.05f));
  EXPECT_TRUE(IsUsableDelta(0.0f));
  EXPECT_FALSE(IsUsableDelta(-0.05f));
  EXPECT_FALSE(IsUsableDelta(std::numeric_limits<float>::infinity()));
  EXPECT_FALSE(IsUsableDelta(std::numeric_limits<float>::quiet_NaN()));
}
