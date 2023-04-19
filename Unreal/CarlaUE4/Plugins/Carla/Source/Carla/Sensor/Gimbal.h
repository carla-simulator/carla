#pragma once

#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Server/CarlaServerResponse.h"

#include "carla/rpc/GimbalMode.h"

#include "Gimbal.generated.h"


UCLASS()
class CARLA_API AGimbal : public ASensor
{
  GENERATED_BODY()

public:

  using GimbalMode = carla::rpc::GimbalMode;

  AGimbal(const FObjectInitializer &ObjectInitializer);

  static FActorDefinition GetSensorDefinition();

  void Set(const FActorDescription &ActorDescription) override;

  void SetOwner(AActor *Owner) override;

  // void Tick(float DeltaSeconds) override;
  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime) override;

  ECarlaServerResponse SetMode(GimbalMode mode);
  ECarlaServerResponse SetCmd(float roll, float pitch, float yaw);

  // Commanded pitch, roll, & yaw
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Command)
  float pitch_cmd = 0.;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Command)
  float roll_cmd = 0.;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Command)
  float yaw_cmd = 0.;

  // Current pitch, roll, & yaw
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Current)
  float pitch = 0.;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Current)
  float roll = 0.;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Current)
  float yaw = 0.;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attributes)
  int tick_count = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attributes)
  int mode = 0;

  // Vehicle yaw in world frame
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attributes)
  float vehicle_yaw = 0.;

  // Gimbal max rotational speed in degrees / second
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attributes)
  float speed = 30.;
private:

  // GimbalMode mode = GimbalMode::WorldRelative;




  /*
  // UPROPERTY()
  // UBoxComponent *Box = nullptr;
  */
};
