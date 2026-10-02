// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Autoware/Game/AutowareGameModeBase.h"

#include "Carla.h"
#include "Carla/Autoware/Game/AutowareWorldSettings.h"
#include "Carla/Game/CarlaEpisode.h"

#include <util/disable-ue4-macros.h>
#include <carla/geom/GeoProjection.h>
#include <util/enable-ue4-macros.h>

AAutowareGameModeBase::AAutowareGameModeBase(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
}

void AAutowareGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
  Super::InitGame(MapName, Options, ErrorMessage);
}

void AAutowareGameModeBase::LoadGeoReference()
{
  auto* WS = Cast<AAutowareWorldSettings>(GetWorld()->GetWorldSettings());

  if (!IsValid(WS))
  {
    Super::LoadGeoReference(); // Fallback to ACarlaGameModeBase::LoadGeoReference default
    return;
  }

  UE_LOG(LogCarla, Log, TEXT("Autoware Settings fetch succeeded."));
  auto* Data = WS->MgrsDataAssetSoftPtr.LoadSynchronous();

  if (!IsValid(Data))
  {
    return;
  }

  // tier4 stored a plain GeoLocation reference on the episode; our branch
  // migrated the georeference to carla::geom::GeoProjection. A Transverse
  // Mercator projection centered on the asset's reference point over the
  // default (spherical) ellipsoid reproduces the legacy GeoLocation::Transform
  // behavior.
  Episode->MapGeoProjection = carla::geom::GeoProjection::Make(
      carla::geom::TransverseMercatorParams(
          Data->GeoReference.Latitude,
          Data->GeoReference.Longitude,
          1.0, 0.0, 0.0,
          carla::geom::Ellipsoid()));

  UE_LOG(LogCarla, Log, TEXT("MGRS georeference loaded successfully."));
  StoreSpawnPoints();
}
