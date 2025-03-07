#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SampleBufferIO.h"

#include "Carla/Sensor/Sensor.h"
#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"

#include "AudioSensorActor.generated.h"

UCLASS()
class AUDIOSENSOR_API AAudioSensorActor : public ASensor
{
    GENERATED_BODY()

public:	

    AAudioSensorActor(const FObjectInitializer &ObjectInitializer);

    static FActorDefinition GetSensorDefinition();

    void SetChunkDuration(double Duration);

    void Set(const FActorDescription &ActorDescription) override;

    void SetOwner(AActor *Owner) override;

    void PrePhysTick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;

public:	

    UFUNCTION(BlueprintCallable)
    void StartRecording();

    UFUNCTION(BlueprintCallable)
    void StopRecording();

    UFUNCTION(BlueprintCallable)
    void Activate();

    UFUNCTION(BlueprintCallable)
    void Deactivate();

protected:
    UPROPERTY(BlueprintReadWrite)
    class USoundSubmix* MasterSubmix;

    UPROPERTY(BlueprintReadOnly)
    bool bIsRecording;

    UPROPERTY(BlueprintReadWrite)
    float ChunkDuration; // in seconds

    UPROPERTY(BlueprintReadOnly)
    bool Active;

    Audio::TSampleBuffer<int16> AudioBuffer;

public:
    UPROPERTY(BlueprintReadOnly)
    FString OutputName;
};
