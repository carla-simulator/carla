// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

enum class ECarlaServerResponse
{
  Success,
  ActorNotFound,
  ComponentNotFound,
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

FString CarlaGetStringError(ECarlaServerResponse Response);
