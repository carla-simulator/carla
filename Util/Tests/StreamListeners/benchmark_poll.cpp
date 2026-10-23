// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include <carla/streaming/detail/MultiStreamState.h>

bool PollStreamListener(carla::streaming::detail::MultiStreamState &state) {
  return state.AreClientsListening();
}
