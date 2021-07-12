// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

enum class ECarlaServerResponse
{
  Success,
  ActorNotFound,
  ActorTypeMismatch,
  FunctionNotSupported,
  NullActor,
  MissingActor,
  NotAVehicle,
  WalkerDead,
  NotAWalker,
  WalkerIncompatibleController,
  AutoPilotNotSupported,
  CarSimPluginNotEnabled,
  NotATrafficLight,
  FunctionNotAvailiableWhenDormant
};

static FString GetStringError(ECarlaServerResponse Response);
