// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "CoreMinimal.h"

#include "Stats/StatsData.h"

#include "BenchmarkSensor.generated.h"

// TODO: disable stats rendering

/*
* As IItemFilter says.
* Some of the FStatsThreadState data methods allow filtering.
*/
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
  // LastFullFrameProcessed is a protected variable in FStatsThreadState
  // and there are not functions to retrieve the value
  // This is an easy hack to get the value that we need
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

  void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;

private:

  // Collects all the data provided by stat unit command and returns them as a FString
  // Eg: FrameTime: 33.33, GameThreadTime: 33.33, RenderThreadTime: 33.33, GPUFrameTime: 33.33, RHITime: 33.33
  FString CollectStatUnit();

  // Collects the data of each stat in each STATGROUP and parse the result into
  // the Ouput FString. Also returns the number of the last valid frame that was
  // used to read the data
  int64 CollectFrameStats(FString& Output);

  // For each stat in the STATGROUP, collect its data and parse to the final FString
  // that will be returned
  FString CollectStatsFromGroup(
    const FStatsThreadStateOverlay& StatsThread,
    const FName& GroupName,
    const TSet<FName>& StatNames,
    int64 Frame);

   // Convert stat comand to STATGROUP
   // Eg: stat sceneredering -> STATGROUP_SceneRendering
  FString ConvertStatCommandToStatGroup(FString StatCmd);

  // Contains the stat command (STATGROUP) as a key and
  // the value is a TSet containing the stats inside that group
  TMap<FName, TSet<FName>> Queries;

#if WITH_EDITOR
  void DumpStatGroups(const FStatsThreadStateOverlay& StatsThread);
#endif // WITH_EDITOR

};
