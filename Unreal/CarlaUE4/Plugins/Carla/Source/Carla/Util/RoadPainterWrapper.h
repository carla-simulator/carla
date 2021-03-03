// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "RoadPainterWrapper.generated.h"

UCLASS()
class CARLA_API ARoadPainterWrapper : public AActor
{
  GENERATED_BODY()

public:

  ARoadPainterWrapper();

  UFUNCTION(BlueprintImplementableEvent, Category = "ARoadPainterWrapper")
	  void ZSizeEvent();

  //Paint events
  UFUNCTION(BlueprintImplementableEvent, Category = "ARoadPainterWrapper")
	  void PaintByActorEvent();

  UFUNCTION(BlueprintImplementableEvent, Category = "ARoadPainterWrapper")
	  void PaintOverSquareEvent();

  UFUNCTION(BlueprintImplementableEvent, Category = "ARoadPainterWrapper")
	  void PaintOverCircleEvent();

  UFUNCTION(BlueprintImplementableEvent, Category = "ARoadPainterWrapper")
	  void PaintAllRoadsEvent();

  //Spawn events
  UFUNCTION(BlueprintImplementableEvent, Category = "ARoadPainterWrapper")
	  void SpawnMeshesByActorEvent();

  UFUNCTION(BlueprintImplementableEvent, Category = "ARoadPainterWrapper")
	  void SpawnMeshesEvent();

  UFUNCTION(BlueprintImplementableEvent, Category = "ARoadPainterWrapper")
	  void SpawnDecalsEvent();

  //Clear events
  UFUNCTION(BlueprintImplementableEvent, Category = "ARoadPainterWrapper")
	  void ClearMaterialEvent();

  UFUNCTION(BlueprintImplementableEvent, Category = "ARoadPainterWrapper")
	  void ClearMaterialByActorEvent();

  UFUNCTION(BlueprintImplementableEvent, Category = "ARoadPainterWrapper")
	  void ClearAllEvent();
};
