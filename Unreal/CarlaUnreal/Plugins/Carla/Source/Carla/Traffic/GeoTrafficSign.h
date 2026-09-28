// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Traffic/TrafficSignBase.h"
#include "GeoTrafficSign.generated.h"

/**
 * A traffic sign baked into a level from the regional sign catalog: a pole and a plate
 * whose material is a constant instance of the atlas selector with the cell baked in.
 *
 * Being an ATrafficSignBase, ATrafficLightManager::SpawnSignals adopts it for the
 * OpenDRIVE signal it stands at (within 5 m, TrafficSignState agreeing with the signal
 * type/subtype) and attaches the matching USignComponent, so the map gets the regional
 * look without a duplicate stock blueprint at the same spot.
 */
UCLASS()
class CARLA_API AGeoTrafficSign : public ATrafficSignBase
{
  GENERATED_BODY()

public:
  AGeoTrafficSign(const FObjectInitializer &ObjectInitializer);

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Geo Sign")
  UStaticMeshComponent* Pole;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Geo Sign")
  UStaticMeshComponent* Plate;

  /// Catalog identifier of the plate (e.g. "max_speed_30").
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geo Sign")
  FString SignName;

  /// Regional style ("VC", "MUTCD", "GB").
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geo Sign")
  FString Style;

  /// OpenDRIVE signal id this sign was baked for (empty for decorative signs).
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geo Sign")
  FString SignalId;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geo Sign")
  FString XodrType;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geo Sign")
  FString XodrSubtype;

  /// Assign meshes / material and place the plate. PlateOffset is relative to the actor
  /// (the pole base), in cm; PlateYaw in degrees turns the plate on the pole.
  UFUNCTION(BlueprintCallable, Category = "Geo Sign")
  void Setup(UStaticMesh* PoleMesh, UStaticMesh* PlateMesh, UMaterialInterface* PlateMaterial,
             FVector PlateOffset, float PlateYaw, float PlateScale);

  /// The ETrafficSignState the traffic-light manager matches against for an OpenDRIVE
  /// signal type/subtype: stop, yield, or the speed-limit state for the subtype (km/h;
  /// ETrafficSignState::SpeedLimit for values without a dedicated state — then also call
  /// SetSpeedLimitKmh, or use ConfigureForSignal). UNKNOWN for anything else.
  UFUNCTION(BlueprintCallable, Category = "Geo Sign")
  static ETrafficSignState StateForSignal(const FString& Type, const FString& Subtype);

  /// Set XodrType / XodrSubtype and the matching runtime state (speed limit in km/h from
  /// the subtype) so ATrafficLightManager adopts this actor for the signal.
  UFUNCTION(BlueprintCallable, Category = "Geo Sign")
  void ConfigureForSignal(const FString& Type, const FString& Subtype);

  /// Levels baked before every km/h value had a state carry UNKNOWN for e.g. 10 km/h signs;
  /// derive the state from XodrType / XodrSubtype so they are adopted without a rebake.
  virtual void PostInitializeComponents() override;
};
