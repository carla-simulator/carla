// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaServerResponse.h"

FString CarlaGetStringError(ECarlaServerResponse Response)
{
  switch (Response)
  {
    case ECarlaServerResponse::Success:
      return "Sucess";
    case ECarlaServerResponse::ActorNotFound:
      return "Actor could not be found in the registry";
    case ECarlaServerResponse::ActorTypeMismatch:
      return "Actor does not match the expected type";
    case ECarlaServerResponse::MissingActor:
      return "Actor is missing";
    case ECarlaServerResponse::NotAVehicle:
      return "Actor is not a Vehicle";
    case ECarlaServerResponse::WalkerDead:
      return "Walker is already dead";
    case ECarlaServerResponse::NotAWalker:
      return "Actor is not a Walker";
    case ECarlaServerResponse::WalkerIncompatibleController:
      return "Walker has incompatible controller";
    case ECarlaServerResponse::NullActor:
      return "Actor is null";
    case ECarlaServerResponse::AutoPilotNotSupported:
      return "Autopilot is not supported by the Actor";
    case ECarlaServerResponse::CarSimPluginNotEnabled:
      return "CarSim plugin is not enabled";
    case ECarlaServerResponse::NotATrafficLight:
      return "Actor is not a traffic light";
    case ECarlaServerResponse::FunctionNotAvailiableWhenDormant:
      return "Function not availiable when the actor is dormant";
  }
  return "unknown error";
}
