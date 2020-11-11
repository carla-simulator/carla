// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"

#include "Stats/StatsData.h"

#include <carla/rpc/Benchmark.h>

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

struct DynamicStats
{
  int64 Frame;

};

struct StaticStats
{
  int32 NumMeshesOnLevel;
  int32 NumDecalsOnLevel;
  int32 NumLightsOnLevel;

};

struct BenchmarkSnapshot
{
  FName MapName;
  StaticStats Statics;
  TArray<DynamicStats> Frame;
};

class CARLA_API FStatsThreadStateOverlay : public FStatsThreadState
{
public:
  int64 GetLastFullFrameProcessed()
  {
    return LastFullFrameProcessed;
  }
};

class CARLA_API BenchmarkAgent
{
public:
  BenchmarkAgent();
  ~BenchmarkAgent();

  using StatsReturnType = std::map<std::string, carla::rpc::BenchmarkQueryValue>;
  using StatsQueriesType = std::multimap<std::string, std::string>;

  StatsReturnType CollectFrameStats(const StatsQueriesType& queries);

private:

  void DumpStatGroups(const FStatsThreadStateOverlay& StatsThread);

  void CollectStatsFromGroup(
    const FStatsThreadStateOverlay& StatsThread,
    const FName& GroupName,
    const TSet<FName>& StatNames,
    int64 Frame);


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
