#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Carla/Sensor/Sensor.h"
#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"

#include "CarlaTemplateExternalInterfaceActor.generated.h"

UCLASS()
class CARLATEMPLATEEXTERNALINTERFACE_API ACarlaTemplateExternalInterfaceActor : public ASensor
{
    GENERATED_BODY()

public:	

    ACarlaTemplateExternalInterfaceActor(const FObjectInitializer &ObjectInitializer);

    void SetSensorTick(double SensorTick);

    void SetTemplateValue(double Value);

    void Set(const FActorDescription &ActorDescription) override;

    void SetOwner(AActor *Owner) override;

protected:
    virtual void BeginPlay() override;

public:	
    UFUNCTION(BlueprintCallable)
    void Activate();

    UFUNCTION(BlueprintCallable)
    void Deactivate();

    UFUNCTION()
    void TemplateFrameCallback();

protected:
    FTimerHandle TemplateFrameTimer;

    FTimerDelegate TemplateFrameDelegate;

    UPROPERTY(BlueprintReadWrite)
    float TemplateValue = 0.25;

    UPROPERTY(BlueprintReadWrite)
    float SensorTick = 0.05; //20HZ

    UPROPERTY(BlueprintReadOnly)
    bool Active = true;

public:
    UPROPERTY(BlueprintReadOnly)
    FString OutputName;
};
