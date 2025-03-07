#pragma once

#include <util/ue-header-guard-begin.h>
#include "Kismet/BlueprintFunctionLibrary.h"
#include <util/ue-header-guard-end.h>

#include "AudioSensorBlueprintLibrary.generated.h"

class AAudioSensorActor;

UCLASS()
class AUDIOSENSOR_API UAudioSensorBlueprintLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

  public: 
  UFUNCTION(BlueprintCallable)
  static bool SaveSoundWaveToWavFile(USoundWave* SoundWave, const FString& FilePath);

  UFUNCTION(BlueprintCallable)
  static FActorDefinition MakeAudioSensorDefinition(TSubclassOf<AAudioSensorActor> Class);

  static void MakeAudioSensorDefinition(bool &Success, FActorDefinition &Definition, TSubclassOf<AAudioSensorActor> Class);

  UFUNCTION(BlueprintCallable)
  static void SetAudioSensor(const FActorDescription &Description, AAudioSensorActor *AudioSensor);

  // Note that this function should really be in a seperate plugin that all custom sensors depend on.
  UFUNCTION(BlueprintCallable)
  static FActorSpawnResult SpawnSensorActor(const AActor* FactoryActor, const FTransform &Transform, const FActorDescription &Description);
};