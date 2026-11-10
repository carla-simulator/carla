// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
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
#include "Carla/Game/FrameData.h"
#include "Carla/Sensor/SensorManager.h"

#include <util/disable-ue4-macros.h>
#include <carla/geom/BoundingBox.h>
#include <carla/geom/GeoProjection.h>
#if WITH_ROS2
    #include <carla/ros2/ROS2.h>
#endif
#include <carla/rpc/Actor.h>
#include <carla/rpc/ActorDescription.h>
#include <carla/rpc/OpendriveGenerationParameters.h>
#include <carla/streaming/Server.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "GameFramework/Pawn.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include <util/ue-header-guard-end.h>

#include "CarlaEpisode.generated.h"

class AWalkerBase;

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

  /// Load a new map and start a new episode.
  ///
  /// If @a MapString is empty, the current map is reloaded.
  UFUNCTION(BlueprintCallable)
  bool LoadNewEpisode(const FString &MapString, bool ResetSettings = true);

  /// Load a new map generating the mesh from OpenDRIVE data and
  /// start a new episode.
  ///
  /// If @a MapString is empty, it fails.
  bool LoadNewOpendriveEpisode(
      const FString &OpenDriveString,
      const carla::rpc::OpendriveGenerationParameters &Params);

  // ===========================================================================
  // -- Episode settings -------------------------------------------------------
  // ===========================================================================

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

  /// Return the unique id of this episode.
  auto GetId() const
  {
    return Id;
  }

  /// Return the name of the map loaded in this episode.
  UFUNCTION(BlueprintCallable)
  /// Block (game thread, bounded) until the RecastBuilder run started by the
  /// last generate_opendrive_world has finished. Returns whether the navmesh
  /// was built (true when no build is pending).
  static bool WaitForPendingNavigationBuild(double TimeoutSeconds);

  /// Delete Saved/OpenDrive|Nav/OpenDriveMap.* left by a previous generated
  /// world (server startup and before each generation).
  static void ClearGeneratedWorldFiles();

  const FString &GetMapName() const
  {
    return MapName;
  }

  /// Game seconds since the start of this episode.
  double GetElapsedGameTime() const
  {
    return ElapsedGameTime;
  }

  /// Visual game seconds
  double GetVisualGameTime() const
  {
    return VisualGameTime;
  }

  void SetVisualGameTime(double Time)
  {
    VisualGameTime = Time;

    // update time in material parameters also
    if (MaterialParameters)
    {
      MaterialParameters->SetScalarParameterValue(FName("VisualTime"), VisualGameTime);
    }
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

  /// Return the geo projection of the map loaded
  const carla::geom::GeoProjection &GetGeoProjection() const
  {
    return MapGeoProjection;
  }

  // ===========================================================================
  // -- Retrieve special actors ------------------------------------------------
  // ===========================================================================

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

  FActorRegistry &GetActorRegistry()
  {
    return ActorDispatcher->GetActorRegistry();
  }

  // ===========================================================================
  // -- Actor look up methods --------------------------------------------------
  // ===========================================================================

  /// Find a Carla actor by id.
  ///
  /// If the actor is not found or is pending kill, the returned view is
  /// invalid.
  FCarlaActor* FindCarlaActor(FCarlaActor::IdType ActorId)
  {
    return ActorDispatcher->GetActorRegistry().FindCarlaActor(ActorId);
  }

  /// Find the actor view of @a Actor.
  ///
  /// If the actor is not found or is pending kill, the returned view is
  /// invalid.
  FCarlaActor* FindCarlaActor(AActor *Actor) const
  {
    return ActorDispatcher->GetActorRegistry().FindCarlaActor(Actor);
  }

  /// Get the description of the Carla actor (sensor) using specific stream id.
  ///
  /// If the actor is not found returns an empty string
  FString GetActorDescriptionFromStream(carla::streaming::detail::stream_id_type StreamId)
  {
    return ActorDispatcher->GetActorRegistry().GetDescriptionFromStream(StreamId);
  }

  /// Find the Carla actor (sensor) that owns the stream @a StreamId, or nullptr.
  FCarlaActor* FindCarlaActorFromStream(carla::streaming::detail::stream_id_type StreamId)
  {
    return ActorDispatcher->GetActorRegistry().FindCarlaActorFromStream(StreamId);
  }

  // ===========================================================================
  // -- Actor handling methods -------------------------------------------------
  // ===========================================================================

  /// Spawns an actor based on @a ActorDescription at @a Transform. To properly
  /// despawn an actor created with this function call DestroyActor.
  ///
  /// @return A pair containing the result of the spawn function and a view over
  /// the actor and its properties. If the status is different of Success the
  /// view is invalid.
  TPair<EActorSpawnResultStatus, FCarlaActor*> SpawnActorWithInfo(
      const FTransform &Transform,
      FActorDescription thisActorDescription,
      FCarlaActor::IdType DesiredId = 0);

  /// Spawns an actor based on @a ActorDescription at @a Transform.
  ///
  /// @return the actor to be spawned
  AActor* ReSpawnActorWithInfo(
      const FTransform &Transform,
      FActorDescription thisActorDescription)
  {
    FTransform NewTransform = Transform;
    auto result = ActorDispatcher->ReSpawnActor(NewTransform, thisActorDescription);
    if (Recorder->IsEnabled())
    {
      // do something?
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
    return SpawnActorWithInfo(Transform, std::move(ActorDescription)).Value->GetActor();
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
    FCarlaActor* CarlaActor = FindCarlaActor(Actor);
    if (CarlaActor)
    {
      carla::rpc::ActorId ActorId = CarlaActor->GetActorId();
      return DestroyActor(ActorId);
    }
    return false;
  }

  bool DestroyActor(carla::rpc::ActorId ActorId)
  {
    if (bIsPrimaryServer)
    {
      GetFrameData().AddEvent(
          CarlaRecorderEventDel{ActorId});
    }
    if (Recorder->IsEnabled())
    {
      // recorder event
      CarlaRecorderEventDel RecEvent{ActorId};
      Recorder->AddEvent(std::move(RecEvent));
    }

    return ActorDispatcher->DestroyActor(ActorId);
  }

  /// SerializeActor, with the ids the primary's clients know on a Multi-GPU
  /// secondary (for data a sensor streams to them).
  carla::rpc::Actor SerializeActorWithPrimaryIds(AActor *Actor) const;

  /// ROS 2: registers the ros_name of each of @a Child's ancestors as one of
  /// its parent frames.
  void AddRosParentNames(FCarlaActor &Child);

  void PutActorToSleep(carla::rpc::ActorId ActorId)
  {
    ActorDispatcher->PutActorToSleep(ActorId, this);
  }

  void WakeActorUp(carla::rpc::ActorId ActorId)
  {
    ActorDispatcher->WakeActorUp(ActorId, this);
  }

  // ===========================================================================
  // -- Other methods ----------------------------------------------------------
  // ===========================================================================

  /// Create a serializable object describing the actor.
  carla::rpc::Actor SerializeActor(FCarlaActor* CarlaActor) const;

  /// Create a serializable object describing the actor.
  /// Can be used to serialized actors that are not in the registry
  carla::rpc::Actor SerializeActor(AActor* Actor) const;

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

  std::string StartRecorder(std::string name, bool AdditionalData, bool StopReplayer);

  FIntVector GetCurrentMapOrigin() const { return CurrentMapOrigin; }

  void SetCurrentMapOrigin(const FIntVector& NewOrigin) { CurrentMapOrigin = NewOrigin; }

  FFrameData& GetFrameData() { return FrameData; }

  /// Multi-GPU primary: the world state the frames sent so far carried.
  FPrimaryWorldState& GetMultiGpuWorldState() { return MultiGpuWorldState; }

  /// A scene light changed: Multi-GPU secondaries get its new state.
  void MarkSceneLightChanged(int32 LightId)
  {
    if (bIsPrimaryServer)
    {
      MultiGpuWorldState.ChangedLights.Add(LightId);
    }
  }

  FSensorManager& GetSensorManager() { return SensorManager; }

  // ===========================================================================
  // -- Pedestrian navigation state (server-side walker navigation) -------------
  // ===========================================================================

  /// Probability that a walker whose navigation starts after this call is a
  /// "crosser" (may traverse crosswalk/road nav areas). Legacy semantics of
  /// carla.World.set_pedestrians_cross_factor.
  void SetPedestriansCrossFactor(float InCrossFactor)
  {
    PedestriansCrossFactor = FMath::Clamp(InCrossFactor, 0.0f, 1.0f);
  }

  /// Seed the RNG behind the crosser draw (and any future navigation
  /// randomness). Deterministic given the same seed and start order.
  void SetPedestriansSeed(uint32 InSeed)
  {
    PedestriansNavRNG.Initialize(static_cast<int32>(InSeed));
  }

  /// Draw once per walker at navigation start: crosser or not.
  bool DrawWalkerIsCrosser()
  {
    return PedestriansNavRNG.FRand() < PedestriansCrossFactor;
  }

  bool bIsPrimaryServer = true;

private:

  friend class ACarlaGameModeBase;
  friend class AAutowareGameModeBase;
  friend class FCarlaEngine;

  void InitializeAtBeginPlay();

  void EndPlay();

  /// True if the centre of @a Walker's capsule is inside the collision of a
  /// building or wall, where physics would push the walker out through the
  /// terrain (#10212).
  bool IsWalkerInsideBuilding(const AWalkerBase &Walker) const;

  void RegisterActorFactory(ACarlaActorFactory &ActorFactory)
  {
    ActorDispatcher->Bind(ActorFactory);
  }

  std::pair<int, FCarlaActor&> TryToCreateReplayerActor(
    FVector &Location,
    FVector &Rotation,
    FActorDescription &ActorDesc,
    unsigned int desiredId);

  bool SetActorSimulatePhysics(FCarlaActor &CarlaActor, bool bEnabled);

  bool SetActorCollisions(FCarlaActor &CarlaActor, bool bEnabled);

  bool SetActorDead(FCarlaActor &CarlaActor);

  void TickTimers(float DeltaSeconds)
  {
    ElapsedGameTime += DeltaSeconds;
    SetVisualGameTime(VisualGameTime + DeltaSeconds);
    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
      ROS2->SetTimestamp(GetElapsedGameTime());
    #endif

  }

  /// Multi-GPU secondary: adopt the primary's clocks for the frame being
  /// played. Absolute, not accumulated, so frames a secondary never played
  /// (while it was loading) cause no drift.
  void SetTimers(double InElapsedGameTime, double InVisualGameTime)
  {
    ElapsedGameTime = InElapsedGameTime;
    SetVisualGameTime(InVisualGameTime);
    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
      ROS2->SetTimestamp(GetElapsedGameTime());
    #endif
  }

  const uint64 Id = 0u;

  // simulation time
  double ElapsedGameTime = 0.0;

  // visual time (used by clounds and other FX that need to be deterministic)
  double VisualGameTime = 0.0;

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

  UPROPERTY(VisibleAnywhere)
  UMaterialParameterCollectionInstance *MaterialParameters = nullptr;

  ACarlaRecorder *Recorder = nullptr;

  carla::geom::GeoProjection MapGeoProjection;

  FIntVector CurrentMapOrigin;

  float PedestriansCrossFactor = 0.0f;

  FRandomStream PedestriansNavRNG = FRandomStream(0);

  FFrameData FrameData;

  FPrimaryWorldState MultiGpuWorldState;

  FSensorManager SensorManager;
};

FString CarlaGetRelevantTagAsString(
    const TSet<carla::rpc::CityObjectLabel> &SemanticTags);
