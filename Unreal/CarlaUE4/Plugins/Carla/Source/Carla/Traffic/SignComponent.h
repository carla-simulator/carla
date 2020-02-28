// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Carla/OpenDrive/OpenDrive.h"
#include "SignComponent.generated.h"

/// Class representing an OpenDRIVE Signal
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API USignComponent : public USceneComponent
{
  GENERATED_BODY()

public:
  USignComponent();

  UFUNCTION(BlueprintPure)
  const FString &GetSignId() const;

  UFUNCTION(BlueprintCallable)
  void SetSignId(const FString &Id);

protected:
  // Called when the game starts
  virtual void BeginPlay() override;

  // Called every frame
  virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

  UPROPERTY(Category = "Traffic Sign", EditAnywhere)
  FString SignId;

};
