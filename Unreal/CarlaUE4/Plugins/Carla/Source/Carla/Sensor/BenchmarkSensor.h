// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "CoreMinimal.h"

#include "Stats/StatsData.h"

#include <carla/rpc/Benchmark.h>

#include "BenchmarkSensor.generated.h"

struct FGroupFilter : public IItemFilter
{
  const TSet<FName>& Items;

  FGroupFilter(const TSet<FName>& InItems)
      : Items(InItems)
  {
  }

  bool Keep(const FStatMessage& Item) override
  {
    // TODO: RawName is faster
    const FName MessageName = Item.NameAndInfo.GetShortName(); //GetRawName();
    return Items.Contains(MessageName);
  }
};

class CARLA_API FStatsThreadStateOverlay : public FStatsThreadState
{
public:
  int64 GetLastFullFrameProcessed()
  {
    return LastFullFrameProcessed;
  }
};

UCLASS()
class CARLA_API ABenchmarkSensor : public ASensor
{
  GENERATED_BODY()

public:

  ABenchmarkSensor(const FObjectInitializer &ObjectInitializer);
  ~ABenchmarkSensor() {}

  static FActorDefinition GetSensorDefinition();

  void Set(const FActorDescription &Description) override;

  void SetQueries(FString InQueries);


protected:

  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  void Tick(float DeltaTime) override;

private:

  FString CollectStatUnit();

  int64 CollectFrameStats(FString& Output);

  void DumpStatGroups(const FStatsThreadStateOverlay& StatsThread);

  void CollectStatsFromGroup(
    const FStatsThreadStateOverlay& StatsThread,
    const FName& GroupName,
    const TSet<FName>& StatNames,
    int64 Frame);

  TMap<FName, TSet<FName>> Queries;

  //FString Queries;

/*
  TMap<FName, FName> ShortToRawNameMap;
  TMap<FName, TSet<FName>> Queries =
  {
    // RenderCore.cpp
    {"STATGROUP_SceneRendering",  { "STAT_TotalGPUFrameTime",
                                    // DrawCalls
                                    "STAT_MeshDrawCalls",
                                    // Lights
                                    "STAT_SceneLights",
                                    // Shadows
                                    // Decals
                                    "STAT_SceneDecals"
                                    "STAT_Decals",
                                    "STAT_DecalsDrawTime",
                                  }
    },
    {"STATGROUP_InitViews",       { "STAT_ViewVisibilityTime",
                                    "STAT_FrustumCull",
                                    "STAT_OcclusionCull",
                                    "STAT_SoftwareOcclusionCull",

                                    "STAT_ProcessedPrimitives",
                                    "STAT_CulledPrimitives",
                                  }
    }
  };
*/
  bool once = true;

};
