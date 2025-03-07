#pragma once

#include "AudioSensorBlueprintLibrary.h"
#include "AudioSensorActor.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla.h"
#include "Carla/Server/CarlaServer.h"
#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Sensor/Sensor.h"

struct FWavHeader
{
    char RIFF[4] = {'R', 'I', 'F', 'F'};
    uint32 FileSize;
    char WAVE[4] = {'W', 'A', 'V', 'E'};
    char fmt[4] = {'f', 'm', 't', ' '};
    uint32 Subchunk1Size = 16; // PCM header size
    uint16 AudioFormat = 1;   // PCM = 1
    uint16 NumChannels;
    uint32 SampleRate;
    uint32 ByteRate;
    uint16 BlockAlign;
    uint16 BitsPerSample;
    char data[4] = {'d', 'a', 't', 'a'};
    uint32 DataSize;
};

#if WITH_EDITOR
#  define CARLA_ABFL_CHECK_ACTOR(ActorPtr)                    \
  if (!IsValid(ActorPtr))     \
  {                                                           \
    UE_LOG(LogAudioSensor, Error, TEXT("Cannot set empty actor!")); \
    return;                                                   \
  }
#else
#  define CARLA_ABFL_CHECK_ACTOR(ActorPtr) \
  IsValid(ActorPtr);
#endif // WITH_EDITOR

bool UAudioSensorBlueprintLibrary::SaveSoundWaveToWavFile(USoundWave* SoundWave, const FString& FilePath)
{
    if (!SoundWave or SoundWave->GetNumChunks() == 0)
    {
        UE_LOG(LogAudioSensor, Error, TEXT("Invalid SoundWave"));
        return false;
    }

    int32 RawDataSize = 0;
    TArray<uint8> RawPCMData;
    for (uint32 i = 0; i < SoundWave->GetNumChunks(); i++){
        uint32 ChunkSize = SoundWave->GetSizeOfChunk(i);
        RawDataSize += ChunkSize;

        uint8* data = new uint8[ChunkSize];
        SoundWave->GetChunkData(i, &data, false);
        
        for (uint32 j = 0; j < ChunkSize; j++){
            RawPCMData.Add(data[j]);
        }
        delete[] data;
    }

    FWavHeader WavHeader;
    WavHeader.NumChannels = SoundWave->NumChannels;
    WavHeader.SampleRate = SoundWave->GetSampleRateForCurrentPlatform();
    WavHeader.BitsPerSample = 16; // Assuming 16-bit PCM
    WavHeader.ByteRate = WavHeader.SampleRate * WavHeader.NumChannels * (WavHeader.BitsPerSample / 8);
    WavHeader.BlockAlign = WavHeader.NumChannels * (WavHeader.BitsPerSample / 8);
    WavHeader.DataSize = RawDataSize;
    WavHeader.FileSize = sizeof(FWavHeader) - 8 + RawDataSize;

    // Combine header and PCM data
    TArray<uint8> WavData;
    WavData.Append(reinterpret_cast<uint8*>(&WavHeader), sizeof(FWavHeader));
    WavData.Append(RawPCMData);

    if (FFileHelper::SaveArrayToFile(WavData, *FilePath))
    {
        UE_LOG(LogAudioSensor, Log, TEXT("Saved WAV file to: %s"), *FilePath);
        return true;
    }
    else
    {
        UE_LOG(LogAudioSensor, Error, TEXT("Failed to save WAV file."));
        return false;
    }
}

FActorDefinition UAudioSensorBlueprintLibrary::MakeAudioSensorDefinition(TSubclassOf<AAudioSensorActor> Class)
{
  FActorDefinition Definition;
  bool Success;
  MakeAudioSensorDefinition(Success, Definition, Class);
  check(Success);
  return Definition;
}

template <typename ... ARGS>
static FString JoinStrings(const FString &Separator, ARGS && ... Args)
{
  return FString::Join(TArray<FString>{std::forward<ARGS>(Args) ...}, *Separator);
}

template <typename ... TStrs>
static void FillIdAndTags(FActorDefinition &Def, TStrs && ... Strings)
{
  Def.Id = JoinStrings(TEXT("."), std::forward<TStrs>(Strings) ...).ToLower();
  Def.Tags = JoinStrings(TEXT(","), std::forward<TStrs>(Strings) ...).ToLower();

  // each actor gets an actor role name attribute (empty by default)
  FActorVariation ActorRole;
  ActorRole.Id = TEXT("role_name");
  ActorRole.Type = EActorAttributeType::String;
  ActorRole.RecommendedValues = { TEXT("default") };
  ActorRole.bRestrictToRecommended = false;
  Def.Variations.Emplace(ActorRole);

  // ROS2
  FActorVariation Var;
  Var.Id = TEXT("ros_name");
  Var.Type = EActorAttributeType::String;
  Var.RecommendedValues = { Def.Id };
  Var.bRestrictToRecommended = false;
  Def.Variations.Emplace(Var);
}

void UAudioSensorBlueprintLibrary::MakeAudioSensorDefinition(
    bool &Success,
    FActorDefinition &Definition,
    TSubclassOf<AAudioSensorActor> Class)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("other"), TEXT("audio"));
  
  Definition.Class = Class;

  FActorVariation Tick;
  Tick.Id = TEXT("sensor_tick");
  Tick.Type = EActorAttributeType::Float;
  Tick.RecommendedValues = { TEXT("0.0") };
  Tick.bRestrictToRecommended = false;

  Definition.Variations.Append({
    Tick});

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("sensor_type"),
    EActorAttributeType::String,
    "audio"});

  Success = UActorBlueprintFunctionLibrary::CheckActorDefinition(Definition);
}

void UAudioSensorBlueprintLibrary::SetAudioSensor(
    const FActorDescription &Description,
    AAudioSensorActor *AudioSensor)
{
  CARLA_ABFL_CHECK_ACTOR(AudioSensor);

  AudioSensor->SetChunkDuration(
      UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat("sensor_tick", Description.Variations, 0.05f)
  );

  AudioSensor->OutputName = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToString("ros_name", Description.Variations, "ABC");

}

FActorSpawnResult UAudioSensorBlueprintLibrary::SpawnSensorActor(
    const AActor* FactoryActor,
    const FTransform &Transform,
    const FActorDescription &Description)
{
  auto *World = FactoryActor->GetWorld();
  if (World == nullptr)
  {
    UE_LOG(LogAudioSensor, Error, TEXT("ACustomSensorFactory: cannot spawn sensor into an empty world."));
    return {};
  }

  UCarlaGameInstance *GameInstance = UCarlaStatics::GetGameInstance(World);
  if (GameInstance == nullptr)
  {
    UE_LOG(LogAudioSensor, Error, TEXT("ACustomSensorFactory: cannot spawn sensor, incompatible game instance."));
    return {};
  }

  auto *Sensor = World->SpawnActorDeferred<ASensor>(
      Description.Class,
      Transform,
      nullptr,
      nullptr,
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  if (Sensor == nullptr)
  {
    UE_LOG(LogAudioSensor, Error, TEXT("ACustomSensorFactory: spawn sensor failed."));
  }
  else
  {
    auto *Episode = GameInstance->GetCarlaEpisode();
    check(Episode != nullptr);

    Sensor->SetEpisode(*Episode);
    Sensor->Set(Description);
    Sensor->SetDataStream(GameInstance->GetServer().OpenStream());
    // ASceneCaptureSensor * SceneCaptureSensor = Cast<ASceneCaptureSensor>(Sensor);
    // if(SceneCaptureSensor)
    // {
    //   SceneCaptureSensor->CameraGBuffers.SceneColor.SetDataStream(GameInstance->GetServer().OpenStream());
    //   SceneCaptureSensor->CameraGBuffers.SceneDepth.SetDataStream(GameInstance->GetServer().OpenStream());
    //   SceneCaptureSensor->CameraGBuffers.SceneStencil.SetDataStream(GameInstance->GetServer().OpenStream());
    //   SceneCaptureSensor->CameraGBuffers.GBufferA.SetDataStream(GameInstance->GetServer().OpenStream());
    //   SceneCaptureSensor->CameraGBuffers.GBufferB.SetDataStream(GameInstance->GetServer().OpenStream());
    //   SceneCaptureSensor->CameraGBuffers.GBufferC.SetDataStream(GameInstance->GetServer().OpenStream());
    //   SceneCaptureSensor->CameraGBuffers.GBufferD.SetDataStream(GameInstance->GetServer().OpenStream());
    //   SceneCaptureSensor->CameraGBuffers.GBufferE.SetDataStream(GameInstance->GetServer().OpenStream());
    //   SceneCaptureSensor->CameraGBuffers.GBufferF.SetDataStream(GameInstance->GetServer().OpenStream());
    //   SceneCaptureSensor->CameraGBuffers.Velocity.SetDataStream(GameInstance->GetServer().OpenStream());
    //   SceneCaptureSensor->CameraGBuffers.SSAO.SetDataStream(GameInstance->GetServer().OpenStream());
    //   SceneCaptureSensor->CameraGBuffers.CustomDepth.SetDataStream(GameInstance->GetServer().OpenStream());
    //   SceneCaptureSensor->CameraGBuffers.CustomStencil.SetDataStream(GameInstance->GetServer().OpenStream());
    // }
  }
  UGameplayStatics::FinishSpawningActor(Sensor, Transform);
  return FActorSpawnResult{Sensor};
}