// Copyright (c) 2020 Robotics and Perception Group (GPR)
// University of Zurich and ETH Zurich
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/SceneCaptureSensor.h"
#include "Carla/Sensor/ShaderBasedSensor.h"

#include <util/disable-ue4-macros.h>
#include <carla/sensor/data/DVSEvent.h>
#include <util/enable-ue4-macros.h>

#include "DVSCamera.generated.h"

namespace dvs
{
  /// DVS Configuration structure
  struct Config
  {
    float Cp;
    float Cm;
    float sigma_Cp;
    float sigma_Cm;
    std::uint64_t refractory_period_ns;
    bool use_log;
    float log_eps;
  };

  inline constexpr std::int64_t secToNanosec(double seconds)
  {
    return static_cast<std::int64_t>(seconds * 1e9);
  }

  inline constexpr double nanosecToSecTrunc(std::int64_t nanoseconds)
  {
    return static_cast<double>(nanoseconds) / 1e9;
  }

} // namespace dvs

/// Sensor that produce Dynamic Vision Events
UCLASS()
class CARLA_API ADVSCamera : public AShaderBasedSensor
{
  GENERATED_BODY()
  using DVSEventArray = std::vector<::carla::sensor::data::DVSEvent>;

public:
  ADVSCamera(const FObjectInitializer &ObjectInitializer);
  static FActorDefinition GetSensorDefinition();
  void Set(const FActorDescription &ActorDescription) override;

protected:
  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime) override;
  void ImageToGray(const TArray<FColor> &image);
  void ImageToLogGray(const TArray<FColor> &image);
  ADVSCamera::DVSEventArray Simulation (float DeltaTime);

private:
  /// Images containing last (current) and previous image
  TArray<float> last_image, prev_image;

  /// Image containing the last reference value to trigger event
  TArray<float> ref_values;

  /// Image containing time of last event in seconds
  TArray<double> last_event_timestamp;

  /// Current time in nanoseconds
  std::int64_t current_time;

  /// DVS simulation configuration
  dvs::Config config;
};
