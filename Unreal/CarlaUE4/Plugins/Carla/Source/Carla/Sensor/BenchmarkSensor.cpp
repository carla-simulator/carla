// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "BenchmarkSensor.h"

static void FillSetOfJsonValues(TSharedPtr<FJsonValue> JsonValue, TSet<FName>& OutStatValues)
{
  EJson JsonType = JsonValue->Type;

  switch(JsonType)
  {
    case EJson::String:
      // UE_LOG(LogCarla, Warning, TEXT("\t String: (%s, %s)"), *It.Key, *JsonValue->AsString());
      OutStatValues.Emplace(*JsonValue->AsString());
      break;
    case EJson::Array:
    {
      const TArray<TSharedPtr<FJsonValue>>& ValuesArray = JsonValue->AsArray();
      // UE_LOG(LogCarla, Warning, TEXT("\t Array: (%s, %d)"), *It.Key, ValuesArray.Num());
      for(const TSharedPtr<FJsonValue>& It2 : ValuesArray)
      {
        FillSetOfJsonValues(It2, OutStatValues);
      }

      break;
    }
    /*
    case EJson::Number:
      UE_LOG(LogCarla, Warning, TEXT("\t Number: (%s, %f)"), *It.Key, JsonValue->AsNumber());
      break;
    case EJson::Boolean:
      UE_LOG(LogCarla, Warning, TEXT("\t Boolean: (%s, %d)"), *It.Key, JsonValue->AsBool());
      break;
    case EJson::Object:
      UE_LOG(LogCarla, Warning, TEXT("\t Object: (%s, _)"), *It.Key);
      break;
    */
    default:
      //UE_LOG(LogCarla, Warning, TEXT("\t Invalid: %d (%s, _)"), JsonType,*It.Key);
      break;
  }
}

ABenchmarkSensor::ABenchmarkSensor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
}

FActorDefinition ABenchmarkSensor::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeBenchmarkSensorDefinition();
}

void ABenchmarkSensor::Set(const FActorDescription &Description)
{
  Super::Set(Description);
  UActorBlueprintFunctionLibrary::SetBenchmarkSensor(Description, this);
}

void ABenchmarkSensor::SetQueries(FString InQueries)
{
  UE_LOG(LogCarla, Error, TEXT("SetQueries %s"), *InQueries);

  TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(InQueries);
  TSharedPtr<FJsonObject> JsonParsed;

  if( !(FJsonSerializer::Deserialize(JsonReader, JsonParsed) && JsonParsed.IsValid()) )
  {
    UE_LOG(LogCarla, Error, TEXT("ABenchmarkSensor::SetQueries INVALID JSON"));
    return;
  }

  UE_LOG(LogCarla, Warning, TEXT("SetQueries Num pairse %d"), JsonParsed->Values.Num());
  for(auto& It :  JsonParsed->Values)
  {
    TSet<FName> StatValues;
    FillSetOfJsonValues(It.Value, StatValues);

    // Convert stat comand to STATGROUP, ie: stat sceneredering -> STATGROUP_SceneRendering
    FName StatGroup = FName(*ConvertStatCommandToStatGroup(It.Key));

    Queries.Emplace(StatGroup, StatValues);

    // Enable command to capture it
    FString Cmd = It.Key;
    GEngine->Exec(GWorld, *Cmd);
  }

  UE_LOG(LogCarla, Warning, TEXT("Final result"));
  for(auto& It : Queries)
  {
    FString Values;
    for(auto& It2 : It.Value)
    {
      Values += *It2.ToString();
      Values += " ";
    }
    UE_LOG(LogCarla, Warning, TEXT("%s -> %s"), *It.Key.ToString(), *Values);
  }

}

void ABenchmarkSensor::BeginPlay()
{
  Super::BeginPlay();
  GEngine->Exec(GWorld, TEXT("stat unit"));

  FStatsThreadStateOverlay& StatsThread = (FStatsThreadStateOverlay&)FStatsThreadState::GetLocalState();
  DumpStatGroups(StatsThread);
}

void ABenchmarkSensor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
  GEngine->Exec(GWorld, TEXT("stat none"));
}

void ABenchmarkSensor::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  FString StatsOutput;
  int64 Frame = CollectFrameStats(StatsOutput);

  FString Output = "";
  Output += FString::Printf(TEXT("{\n  Frame:%lld,\n"), Frame);
  Output += CollectStatUnit();
  Output += StatsOutput;
  Output += "}";

  auto DataStream = GetDataStream(*this);
  DataStream.Send(*this, TCHAR_TO_UTF8(*Output));

  /*
  const TArray<FString>* EnabledStats = World->GetGameViewport()->GetEnabledStats();
  for(const FString& Stat : *EnabledStats)
  {
    UE_LOG(LogCarla, Error, TEXT(" Stat %s"), *Stat);
  }
  */

}

FString ABenchmarkSensor::CollectStatUnit()
{
  FStatUnitData* StatUnitData = GetWorld()->GetGameViewport()->GetStatUnitData();
  float FrameTime = 0.0f;
  float GameThreadTime = 0.0f;
  float RenderThreadTime = 0.0f;
  float GPUFrameTime = 0.0f;
  float RHITTime = 0.0f;

  if(StatUnitData)
  {
    FrameTime = StatUnitData->FrameTime;
    GameThreadTime = StatUnitData->GameThreadTime;
    RenderThreadTime = StatUnitData->RenderThreadTime;
    GPUFrameTime = StatUnitData->GPUFrameTime;
    RHITTime = StatUnitData->RHITTime;
    /*
    UE_LOG(LogCarla, Error, TEXT("Frame: %.2f, Game: %.2f, Draw: %.2f, GPU: %.2f,  RHI: %.2f"),
      StatUnitData->FrameTime, StatUnitData->GameThreadTime, StatUnitData->RenderThreadTime, StatUnitData->GPUFrameTime, StatUnitData->RHITTime);
    */
  }
  return FString::Printf(TEXT("  FrameTime : %.2f,\n  GameThreadTime : %.2f,\n  RenderThreadTime : %.2f,\n  GPUFrameTime : %.2f,\n  RHITime : %.2f,\n"),
                              FrameTime, GameThreadTime, RenderThreadTime, GPUFrameTime, RHITTime);
}

int64 ABenchmarkSensor::CollectFrameStats(FString& Output)
{
  FString Result;

  // Get the reference to the stats thread
  FStatsThreadStateOverlay& StatsThread = (FStatsThreadStateOverlay&)FStatsThreadState::GetLocalState();

  // Get the number of the last processed frame and check if it is valid (just for sure)
  int64 LastGoodGameFrame = StatsThread.GetLastFullFrameProcessed();
  if (StatsThread.IsFrameValid(LastGoodGameFrame) == false)
  {
    return LastGoodGameFrame;
  }

  int NumIt = 0;
  for(auto It : Queries)
  {
    Output += FString::Printf(TEXT("  %s : ["), *It.Key.ToString());
    Output += CollectStatsFromGroup(StatsThread, It.Key, It.Value, LastGoodGameFrame);
    NumIt++;
    Output += (NumIt < Queries.Num() - 1) ? "],\n" : "]\n";
  }

  return LastGoodGameFrame;
}

void ABenchmarkSensor::DumpStatGroups(const FStatsThreadStateOverlay& StatsThread)
{
  FString Output = "";
  const TMultiMap<FName, FName>& Groups = StatsThread.Groups;

  TArray<FName> Keys;
  Groups.GetKeys(Keys);

  for(const FName& Key : Keys)
  {
    TArray<FName> Values;
    Groups.MultiFind(Key, Values);

    Output += Key.ToString() + "\n";

    for(const FName& Value : Values)
    {
      Output += "\t" + Value.ToString() + "\n";
    }
  }

  FString FilePath = FPaths::GameAgnosticSavedDir() + "StatGroups.txt";
  FFileHelper::SaveStringToFile(
    Output,
    *FilePath,
    FFileHelper::EEncodingOptions::AutoDetect,
    &IFileManager::Get(),
    EFileWrite::FILEWRITE_Silent);
}

FString ABenchmarkSensor::CollectStatsFromGroup(
  const FStatsThreadStateOverlay& StatsThread,
  const FName& GroupName,
  const TSet<FName>& StatNames,
  int64 Frame)
{
  FString Output = "";

  // Gather the names of the stats that are in this group.
  TArray<FName> GroupItems;
  StatsThread.Groups.MultiFind(GroupName, GroupItems);

  // Prepare the set of names and raw names of the stats we want to get
  /*
  TSet<FName> EnabledItems;
  for (const FName& ShortName : GroupItems)
  {
    UE_LOG(LogCarla, Error, TEXT("Items %s"), *ShortName.ToString());
    //EnabledItems.Add(ShortName);
    // TODO: RawName is faster

    //const FStatMessage* LongName = StatsThread.ShortNameToLongName.Find(ShortName);
    //if (LongName)
    //{
    //  EnabledItems.Add(LongName->NameAndInfo.GetRawName());
    //}
  }
  */

  // Create a filter (needed by stats gathering function)
  FGroupFilter Filter(StatNames);

  // Create empty stat stack node (needed by stats gathering function)
  FRawStatStackNode HierarchyInclusive;

  // Prepare the array for stat messages
  TArray<FStatMessage> NonStackStats;

  // COLLECT ALL STATS TO THE ARRAY HERE
  StatsThread.UncondenseStackStats(Frame, HierarchyInclusive, &Filter, &NonStackStats);
  //StatsThread.UncondenseStackStats(Frame, HierarchyInclusive, nullptr, &NonStackStats);

  // Go through all stats
  // There are many ways to display them, dig around the code to display it as you want :)
  for (int i = 0; i < NonStackStats.Num(); i++)
  {
    const FStatMessage& Stat = NonStackStats[i];

    // Here we are getting the raw name
    FName StatName = Stat.NameAndInfo.GetShortName(); //GetRawName();

    // Here we are getting values
    switch (Stat.NameAndInfo.GetField<EStatDataType>())
    {
      case EStatDataType::ST_int64:
        {
          int64 Value = Stat.GetValue_int64();
          Output += FString::Printf(TEXT("{%s:%lld}"),  *StatName.ToString(), Value);
        }
        break;
      case EStatDataType::ST_double:
        {
          double Value = Stat.GetValue_double();
          Output += FString::Printf(TEXT("{%s:%f}"),  *StatName.ToString(), Value);
        }
        break;
      case EStatDataType::ST_Ptr:
        {
          uint64 Value = Stat.GetValue_Ptr();
          Output += FString::Printf(TEXT("{%s:%lld}"),  *StatName.ToString(), Value);
        }
        break;
      default:
        UE_LOG(LogCarla, Error, TEXT("Stat: %s is %d"), *StatName.ToString(), static_cast<int32>(Stat.NameAndInfo.GetField<EStatDataType>()));
        check(0);
    }
    if(i < NonStackStats.Num() - 1)
    {
      Output += ", ";
    }
  }
  return Output;
}

FString ABenchmarkSensor::ConvertStatCommandToStatGroup(FString StatCmd)
{
  FString StatGroupResult = "";
  // Split the command (ie: stat scenerendering)
  TArray<FString> StringCmd;
  StatCmd.ParseIntoArray(StringCmd, TEXT(" "), true);
  // 0: stat
  // 1: scenerendering

  // Get all the STATGROUPs
  FStatsThreadStateOverlay& StatsThread = (FStatsThreadStateOverlay&)FStatsThreadState::GetLocalState();
  TArray<FName> Keys;
  StatsThread.Groups.GetKeys(Keys);

  for(const FName& Key : Keys)
  {
    FString StatGroupStr = Key.ToString();
    // STATGROUP_SceneRendering Contains scenerendering; case ignored by default on compare
    if(StatGroupStr.Contains(StringCmd[1]))
    {
      StatGroupResult = StatGroupStr;
      break;
    }
  }
  return StatGroupResult;
}
