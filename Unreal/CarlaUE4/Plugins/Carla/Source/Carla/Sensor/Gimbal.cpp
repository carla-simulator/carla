#include "Carla.h"
#include "Carla/Sensor/Gimbal.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Util/BoundingBoxCalculator.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

int sgn(float val) {
  return (0 < val) - (val < 0);
}

float angleDiff(float a1, float a2) {
  float diff = fmod((a2 - a1 + 180.), 360.) - 180.;
  return diff < -180. ? diff + 360. : diff;
}

// template <typename T> int sgn(T val) {
//     return (T(0) < val) - (val < T(0));
// }

AGimbal::AGimbal(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
}

FActorDefinition AGimbal::GetSensorDefinition()
{
  auto Definition = UActorBlueprintFunctionLibrary::MakeGenericSensorDefinition(
    TEXT("other"),
    TEXT("gimbal"));

  FActorVariation Speed;
  Speed.Id = TEXT("speed");
  Speed.Type = EActorAttributeType::Float;
  Speed.RecommendedValues = { TEXT("30.0") };
  Speed.bRestrictToRecommended = false;

  Definition.Variations.Append({ Speed });

  return Definition;
}

void AGimbal::Set(const FActorDescription &Description)
{
  Super::Set(Description);

  speed = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
    "speed",
    Description.Variations,
    30.0f);

  constexpr float M_TO_CM = 100.0f; // Unit conversion.
}

void AGimbal::SetOwner(AActor *Owner)
{
  Super::SetOwner(Owner);
}

// void AGimbal::Tick(float DeltaSeconds)
void AGimbal::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
  /*
  // TRACE_CPUPROFILER_EVENT_SCOPE(AGimbal::PostPhysTick);
  */

  vehicle_yaw = GetOwner()->GetActorRotation().Yaw;
  float max_rot_this_tick = speed * DeltaTime;

  float dr = angleDiff(roll, roll_cmd);
  roll = roll + sgn(dr) * std::min(std::abs(dr), max_rot_this_tick);

  float dp = angleDiff(pitch, pitch_cmd);
  pitch = pitch + sgn(dp) * std::min(std::abs(dp), max_rot_this_tick);

  float final_yaw_cmd = yaw_cmd;

  // GimbalMode mode_enum = GimbalMode(mode);
  // if (mode_enum == GimbalMode::VehicleRelative) {
  if (mode == 0) {
    final_yaw_cmd = fmod(vehicle_yaw + yaw_cmd, 360.);
  } else if (mode == 1) {
    final_yaw_cmd = yaw_cmd;
  }

  float dy = angleDiff(yaw, final_yaw_cmd);
  yaw = yaw + sgn(dy) * std::min(std::abs(dy), max_rot_this_tick);

  // SetActorRelativeRotation(FRotator(pitch, yaw, roll), false, nullptr, ETeleportType::TeleportPhysics);
  SetActorRotation(FRotator(pitch, yaw, roll), ETeleportType::TeleportPhysics);
}

ECarlaServerResponse AGimbal::SetMode(GimbalMode mode) {
  if (mode == GimbalMode::VehicleRelative) {
    this->mode = 0;
  } else {
    this->mode = 1;
  }

  return ECarlaServerResponse::Success;
}

ECarlaServerResponse AGimbal::SetCmd(float roll, float pitch, float yaw) {
  this->roll_cmd = fmod(roll, 360.);
  this->pitch_cmd = fmod(pitch, 360.);
  this->yaw_cmd = fmod(yaw, 360.);

  return ECarlaServerResponse::Success;
}