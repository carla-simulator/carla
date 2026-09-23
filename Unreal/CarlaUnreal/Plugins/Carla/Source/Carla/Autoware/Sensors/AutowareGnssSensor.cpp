// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Autoware/Sensors/AutowareGnssSensor.h"

#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Autoware/Game/AutowareWorldSettings.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/MapGen/LargeMapManager.h"

#include <util/disable-ue4-macros.h>
#include "carla/geom/Vector3D.h"
#include "carla/ros2/ROS2.h"
#include <util/enable-ue4-macros.h>

AAutowareGnssSensor::AAutowareGnssSensor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));
}

FActorDefinition AAutowareGnssSensor::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeAutowareGnssDefinition();
}

void AAutowareGnssSensor::BeginPlay()
{
  Super::BeginPlay();

  // Load MGRS data from the active level's world settings.
  LoadMgrsData();

  const UCarlaEpisode* episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
  CurrentGeoProjection = episode->GetGeoProjection();
}

void AAutowareGnssSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AAutowareGnssSensor::PostPhysTick);

  FVector ActorLocation = GetActorLocation();
  ALargeMapManager * LargeMap = UCarlaStatics::GetLargeMapManager(GetWorld());
  if (LargeMap)
  {
    ActorLocation = LargeMap->LocalToGlobalLocation(ActorLocation);
  }
  carla::geom::Location Location = ActorLocation;
  carla::geom::GeoLocation CurrentLocation = CurrentGeoProjection.TransformToGeoLocation(Location);

  LatitudeValue = CurrentLocation.latitude;
  LongitudeValue = CurrentLocation.longitude;
  AltitudeValue = CurrentLocation.altitude;

  if (bApplyNoiseError)
  {
    // Compute the noise for the sensor
    const float LatError = RandomEngine->GetNormalDistribution(0.0f, LatitudeDeviation);
    const float LonError = RandomEngine->GetNormalDistribution(0.0f, LongitudeDeviation);
    const float AltError = RandomEngine->GetNormalDistribution(0.0f, AltitudeDeviation);

    // Apply the noise to the sensor
    LatitudeValue += LatitudeBias + LatError;
    LongitudeValue += LongitudeBias + LonError;
    AltitudeValue += AltitudeBias + AltError;
  }

  auto DataStream = GetDataStream(*this);

  // ROS2
  #if defined(WITH_ROS2)
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  if (ROS2->IsEnabled())
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("ROS2 Send");
    auto StreamId = carla::streaming::detail::token_type(GetToken()).get_stream_id();
    AActor* ParentActor = GetAttachParentActor();
    const FTransform SensorWorldTransform = GetActorTransform();
    double MgrsOffsetPosition[3] = {0.0, 0.0, 0.0};
    if (MgrsDataAsset)
    {
      MgrsOffsetPosition[0] = static_cast<double>(MgrsDataAsset->MgrsOffsetPosition.X);
      MgrsOffsetPosition[1] = static_cast<double>(MgrsDataAsset->MgrsOffsetPosition.Y);
      MgrsOffsetPosition[2] = static_cast<double>(MgrsDataAsset->MgrsOffsetPosition.Z);
    }

    if (ParentActor)
    {
      FTransform LocalTransformRelativeToParent = GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform());
      ROS2->ProcessDataFromAutowareGNSS(DataStream.GetSensorType(), StreamId, LocalTransformRelativeToParent, carla::geom::GeoLocation{LatitudeValue, LongitudeValue, AltitudeValue}, SensorWorldTransform, MgrsOffsetPosition, this);
    }
    else
    {
      ROS2->ProcessDataFromAutowareGNSS(DataStream.GetSensorType(), StreamId, DataStream.GetSensorTransform(), carla::geom::GeoLocation{LatitudeValue, LongitudeValue, AltitudeValue}, SensorWorldTransform, MgrsOffsetPosition, this);
    }
  }
  #endif

  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("AAutowareGnssSensor Stream Send");
    DataStream.SerializeAndSend(*this, carla::geom::GeoLocation{LatitudeValue, LongitudeValue, AltitudeValue});
  }
}

void AAutowareGnssSensor::LoadMgrsData()
{
  if (MgrsDataAsset)
  {
    return; // Already loaded
  }

  if (const auto* WS = Cast<AAutowareWorldSettings>(GetWorld()->GetWorldSettings()))
  {
    if (WS->MgrsDataAssetSoftPtr.IsNull())
    {
      UE_LOG(LogCarla, Warning, TEXT("MGRS Data Asset SoftPtr not set in WorldSettings."));
      return;
    }

    UMgrsDataAsset* Data = WS->MgrsDataAssetSoftPtr.Get();
    if (!IsValid(Data))
    {
      Data = WS->MgrsDataAssetSoftPtr.LoadSynchronous();
    }

    MgrsDataAsset = Data;
  }
}

void AAutowareGnssSensor::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  UActorBlueprintFunctionLibrary::SetAutowareGnss(ActorDescription, this);
}

void AAutowareGnssSensor::SetLatitudeDeviation(float Value)
{
  LatitudeDeviation = Value;
}

void AAutowareGnssSensor::SetLongitudeDeviation(float Value)
{
  LongitudeDeviation = Value;
}

void AAutowareGnssSensor::SetAltitudeDeviation(float Value)
{
  AltitudeDeviation = Value;
}

void AAutowareGnssSensor::SetLatitudeBias(float Value)
{
  LatitudeBias = Value;
}

void AAutowareGnssSensor::SetLongitudeBias(float Value)
{
  LongitudeBias = Value;
}

void AAutowareGnssSensor::SetAltitudeBias(float Value)
{
  AltitudeBias = Value;
}

float AAutowareGnssSensor::GetLatitudeDeviation() const
{
  return LatitudeDeviation;
}
float AAutowareGnssSensor::GetLongitudeDeviation() const
{
  return LongitudeDeviation;
}
float AAutowareGnssSensor::GetAltitudeDeviation() const
{
  return AltitudeDeviation;
}

float AAutowareGnssSensor::GetLatitudeBias() const
{
  return LatitudeBias;
}
float AAutowareGnssSensor::GetLongitudeBias() const
{
  return LongitudeBias;
}
float AAutowareGnssSensor::GetAltitudeBias() const
{
  return AltitudeBias;
}

double AAutowareGnssSensor::GetLatitudeValue() const
{
  return LatitudeValue;
}

double AAutowareGnssSensor::GetLongitudeValue() const
{
  return LongitudeValue;
}

double AAutowareGnssSensor::GetAltitudeValue() const
{
  return AltitudeValue;
}

bool AAutowareGnssSensor::IsNoiseErrorEnabled() const
{
  return bApplyNoiseError;
}

void AAutowareGnssSensor::SetNoiseErrorActive(const bool bEnabled)
{
  bApplyNoiseError = bEnabled;
}
