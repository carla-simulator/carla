// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDispatcher.h"
#include "Carla/Recorder/CarlaRecorder.h"
#include "Carla/Sensor/WorldObserver.h"
#include "Carla/Server/CarlaServer.h"
#include "Carla/Settings/EpisodeSettings.h"
#include "Carla/Util/ActorAttacher.h"
#include "Carla/Weather/Weather.h"

#include "GameFramework/Pawn.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/geom/BoundingBox.h>
#include <carla/geom/GeoLocation.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/ActorDescription.h>
#include <carla/streaming/Server.h>
#include <compiler/enable-ue4-macros.h>

#include "CarlaEpisode.generated.h"

/// A simulation episode.
///
/// Each time the level is restarted a new episode is created.
UCLASS(BlueprintType, Blueprintable)
class CARLA_API UCarlaEpisode : public UObject
{
  GENERATED_BODY()

  // ===========================================================================
  // -- Constructor ------------------------------------------------------------
  // ===========================================================================

public:

  UCarlaEpisode(const FObjectInitializer &ObjectInitializer);

  // ===========================================================================
  // -- Load a new episode -----------------------------------------------------
  // ===========================================================================

public:

  /// Load a new map and start a new episode.
  ///
  /// If @a MapString is empty, the current map is reloaded.
  UFUNCTION(BlueprintCallable)
  bool LoadNewEpisode(const FString &MapString);

  /// Load a new map generating the mesh from OpenDRIVE data and
  /// start a new episode.
  ///
  /// If @a MapString is empty, it fails.
  UFUNCTION(BlueprintCallable)
  bool LoadNewOpendriveEpisode(const FString &OpenDriveString);

  // ===========================================================================
  // -- Episode settings -------------------------------------------------------
  // ===========================================================================

public:

  UFUNCTION(BlueprintCallable)
  const FEpisodeSettings &GetSettings() const
  {
    return EpisodeSettings;
  }

  UFUNCTION(BlueprintCallable)
  void ApplySettings(const FEpisodeSettings &Settings);

  // ===========================================================================
  // -- Retrieve info about this episode ---------------------------------------
  // ===========================================================================

public:

  /// Return the unique id of this episode.
  auto GetId() const
  {
    return Id;
  }

  /// Return the name of the map loaded in this episode.
  UFUNCTION(BlueprintCallable)
  const FString &GetMapName() const
  {
    return MapName;
  }

  /// Game seconds since the start of this episode.
  double GetElapsedGameTime() const
  {
    return ElapsedGameTime;
  }

  /// Return the list of actor definitions that are available to be spawned this
  /// episode.
  UFUNCTION(BlueprintCallable)
  const TArray<FActorDefinition> &GetActorDefinitions() const
  {
    return ActorDispatcher->GetActorDefinitions();
  }

  /// Return the list of recommended spawn points for vehicles.
  UFUNCTION(BlueprintCallable)
  TArray<FTransform> GetRecommendedSpawnPoints() const;

  /// Return the GeoLocation point of the map loaded
  const carla::geom::GeoLocation &GetGeoReference() const
  {
    return MapGeoReference;
  }

  // ===========================================================================
  // -- Retrieve special actors ------------------------------------------------
  // ===========================================================================

public:

  UFUNCTION(BlueprintCallable)
  APawn *GetSpectatorPawn() const
  {
    return Spectator;
  }

  UFUNCTION(BlueprintCallable)
  AWeather *GetWeather() const
  {
    return Weather;
  }

  const FActorRegistry &GetActorRegistry() const
  {
    return ActorDispatcher->GetActorRegistry();
  }

  // ===========================================================================
  // -- Actor look up methods --------------------------------------------------
  // ===========================================================================

public:

  /// Find a Carla actor by id.
  ///
  /// If the actor is not found or is pending kill, the returned view is
  /// invalid.
  FActorView FindActor(FActorView::IdType ActorId) const
  {
    return ActorDispatcher->GetActorRegistry().Find(ActorId);
  }

  /// Find the actor view of @a Actor.
  ///
  /// If the actor is not found or is pending kill, the returned view is
  /// invalid.
  FActorView FindActor(AActor *Actor) const
  {
    return ActorDispatcher->GetActorRegistry().Find(Actor);
  }

  /// Find the actor view of @a Actor. If the actor is not found, a "fake" view
  /// is returned emulating an existing Carla actor. Use this to return views
  /// over static actors present in the map.
  ///
  /// If the actor is pending kill, the returned view is invalid.
  FActorView FindOrFakeActor(AActor *Actor) const
  {
    return ActorDispatcher->GetActorRegistry().FindOrFake(Actor);
  }

  // ===========================================================================
  // -- Actor handling methods -------------------------------------------------
  // ===========================================================================

public:

  /// Spawns an actor based on @a ActorDescription at @a Transform. To properly
  /// despawn an actor created with this function call DestroyActor.
  ///
  /// @return A pair containing the result of the spawn function and a view over
  /// the actor and its properties. If the status is different of Success the
  /// view is invalid.
  TPair<EActorSpawnResultStatus, FActorView> SpawnActorWithInfo(
      const FTransform &Transform,
      FActorDescription thisActorDescription,
      FActorView::IdType DesiredId = 0)
  {
    auto result = ActorDispatcher->SpawnActor(Transform, thisActorDescription, DesiredId);
    if (Recorder->IsEnabled())
    {
      if (result.Key == EActorSpawnResultStatus::Success)
      {
        Recorder->CreateRecorderEventAdd(
          result.Value.GetActorId(),
          static_cast<uint8_t>(result.Value.GetActorType()),
          Transform,
          std::move(thisActorDescription)
        );
      }
    }

    return result;
  }

  /// Spawns an actor based on @a ActorDescription at @a Transform. To properly
  /// despawn an actor created with this function call DestroyActor.
  ///
  /// @return nullptr on failure.
  ///
  /// @note Special overload for blueprints.
  UFUNCTION(BlueprintCallable)
  AActor *SpawnActor(
      const FTransform &Transform,
      FActorDescription ActorDescription)
  {
    return SpawnActorWithInfo(Transform, std::move(ActorDescription)).Value.GetActor();
  }

  /// Attach @a Child to @a Parent.
  ///
  /// @pre Actors cannot be null.
  UFUNCTION(BlueprintCallable)
  void AttachActors(
      AActor *Child,
      AActor *Parent,
      EAttachmentType InAttachmentType = EAttachmentType::Rigid);

  /// @copydoc FActorDispatcher::DestroyActor(AActor*)
  UFUNCTION(BlueprintCallable)
  bool DestroyActor(AActor *Actor)
  {
      if (Recorder->IsEnabled())
      {
        // recorder event
        CarlaRecorderEventDel RecEvent
        {
          GetActorRegistry().Find(Actor).GetActorId()
        };
        Recorder->AddEvent(std::move(RecEvent));
      }

    return ActorDispatcher->DestroyActor(Actor);
  }

  // ===========================================================================
  // -- Other methods ----------------------------------------------------------
  // ===========================================================================

public:

  /// Create a serializable object describing the actor.
  carla::rpc::Actor SerializeActor(FActorView ActorView) const;

  // ===========================================================================
  // -- Private methods and members --------------------------------------------
  // ===========================================================================

  ACarlaRecorder *GetRecorder() const
  {
    return Recorder;
  }

  void SetRecorder(ACarlaRecorder *Rec)
  {
    Recorder = Rec;
  }

  CarlaReplayer *GetReplayer() const
  {
    return Recorder->GetReplayer();
  }

  std::string StartRecorder(std::string name);

private:

  friend class ACarlaGameModeBase;
  friend class FCarlaEngine;

  void InitializeAtBeginPlay();

  void EndPlay();

  void RegisterActorFactory(ACarlaActorFactory &ActorFactory)
  {
    ActorDispatcher->Bind(ActorFactory);
  }

  std::pair<int, FActorView&> TryToCreateReplayerActor(
    FVector &Location,
    FVector &Rotation,
    FActorDescription &ActorDesc,
    unsigned int desiredId);

  bool SetActorSimulatePhysics(FActorView &ActorView, bool bEnabled);

  void TickTimers(float DeltaSeconds)
  {
    ElapsedGameTime += DeltaSeconds;
  }

  const uint64 Id = 0u;

  double ElapsedGameTime = 0.0;

  UPROPERTY(VisibleAnywhere)
  FString MapName;

  UPROPERTY(VisibleAnywhere)
  FEpisodeSettings EpisodeSettings;

  UPROPERTY(VisibleAnywhere)
  UActorDispatcher *ActorDispatcher = nullptr;

  UPROPERTY(VisibleAnywhere)
  APawn *Spectator = nullptr;

  UPROPERTY(VisibleAnywhere)
  AWeather *Weather = nullptr;

  ACarlaRecorder *Recorder = nullptr;

  carla::geom::GeoLocation MapGeoReference;
};
