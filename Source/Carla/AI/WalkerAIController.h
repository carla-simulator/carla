// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "AIController.h"
#include "WalkerAIController.generated.h"

class UAISenseConfig_Sight;

UCLASS()
class CARLA_API AWalkerAIController : public AAIController
{
  GENERATED_BODY()

public:

  AWalkerAIController(const FObjectInitializer& ObjectInitializer);

  virtual void Tick(float DeltaSeconds) override;

  virtual FPathFollowingRequestResult MoveTo(
      const FAIMoveRequest& MoveRequest,
      FNavPathSharedPtr* OutPath = nullptr) override;

  UFUNCTION(BlueprintCallable)
  void SenseActors(TArray<AActor *> Actors);

  bool WalkerIsStuck() const
  {
    return bIsStuck;
  }

private:

  void TryResumeMovement();

  void TryPauseMovement();

  UPROPERTY(Category = "Walker AI Controller", VisibleAnywhere)
  UAISenseConfig_Sight *SightConfiguration;

  UPROPERTY(VisibleAnywhere)
  bool bIsPaused = false;

  UPROPERTY(VisibleAnywhere)
  bool bIsStuck = false;
};
