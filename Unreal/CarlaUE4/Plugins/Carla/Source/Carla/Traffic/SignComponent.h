// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Carla/OpenDrive/OpenDrive.h"
#include "SignComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API USignComponent : public UActorComponent
{
  GENERATED_BODY()

public:
  // Sets default values for this component's properties
  USignComponent();

  void SetSignId(carla::road::SignId SignId)
  {
    Id = SignId;
  }

protected:
  // Called when the game starts
  virtual void BeginPlay() override;

public:
  // Called every frame
  virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:
  carla::road::SignId Id;

  FTransform Transform;


};
