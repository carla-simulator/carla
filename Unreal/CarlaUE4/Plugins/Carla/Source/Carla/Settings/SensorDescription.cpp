// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "SensorDescription.h"

#include "Util/IniFile.h"

void USensorDescription::AcceptVisitor(ISensorDescriptionVisitor &Visitor) const
{
  unimplemented();
}

void USensorDescription::Load(const FIniFile &Config, const FString &Section)
{
  constexpr float TO_CENTIMETERS = 1e2;
  Config.GetFloat(*Section, TEXT("PositionX"), Position.X, TO_CENTIMETERS);
  Config.GetFloat(*Section, TEXT("PositionY"), Position.Y, TO_CENTIMETERS);
  Config.GetFloat(*Section, TEXT("PositionZ"), Position.Z, TO_CENTIMETERS);
  Config.GetFloat(*Section, TEXT("RotationPitch"), Rotation.Pitch);
  Config.GetFloat(*Section, TEXT("RotationYaw"), Rotation.Yaw);
  Config.GetFloat(*Section, TEXT("RotationRoll"), Rotation.Roll);
}

void USensorDescription::Log() const
{
  UE_LOG(LogCarla, Log, TEXT("[%s/%s]"), TEXT("CARLA/Sensor"), *Name);
  UE_LOG(LogCarla, Log, TEXT("Id = %d"), GetId());
  UE_LOG(LogCarla, Log, TEXT("Type = %s"), *Type);
  UE_LOG(LogCarla, Log, TEXT("Position = (%s)"), *Position.ToString());
  UE_LOG(LogCarla, Log, TEXT("Rotation = (%s)"), *Rotation.ToString());
}
