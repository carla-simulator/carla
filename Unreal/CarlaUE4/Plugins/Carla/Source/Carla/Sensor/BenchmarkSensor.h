// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "CoreMinimal.h"

#include "Stats/StatsData.h"

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

  void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;

private:

  FString CollectStatUnit();

  int64 CollectFrameStats(FString& Output);

#if WITH_EDITOR
  void DumpStatGroups(const FStatsThreadStateOverlay& StatsThread);
#endif // WITH_EDITOR

  FString CollectStatsFromGroup(
    const FStatsThreadStateOverlay& StatsThread,
    const FName& GroupName,
    const TSet<FName>& StatNames,
    int64 Frame);

  FString ConvertStatCommandToStatGroup(FString StatCmd);

  TMap<FName, TSet<FName>> Queries;

};
