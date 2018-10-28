// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

class UWorld;

namespace carla { namespace rpc { class DebugShape; }}

class FDebugShapeDrawer
{
public:

  explicit FDebugShapeDrawer(UWorld &InWorld) : World(InWorld) {}

  void Draw(const carla::rpc::DebugShape &Shape);

private:

  UWorld &World;
};
