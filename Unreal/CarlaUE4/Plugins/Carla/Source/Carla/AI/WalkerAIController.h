// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "AIController.h"
#include "WalkerAIController.generated.h"

class UAISenseConfig_Sight;

UENUM(BlueprintType)
enum class EWalkerStatus : uint8 {
  Moving                UMETA(DisplayName = "Walker Is Moving"),
  Paused                UMETA(DisplayName = "Walker Movement Is Paused"),
  MoveCompleted         UMETA(DisplayName = "Walker Completed Move"),
  Stuck                 UMETA(DisplayName = "Walker Is Stuck"),
  RunOver               UMETA(DisplayName = "Walker Has Been Run Over"),
  Invalid               UMETA(DisplayName = "Walker Is Invalid"),
  Unknown               UMETA(DisplayName = "Unknown"),
};

UCLASS()
class CARLA_API AWalkerAIController : public AAIController
{
  GENERATED_BODY()

public:

  AWalkerAIController(const FObjectInitializer& ObjectInitializer);

  virtual void Possess(APawn *aPawn) override;

  virtual void Tick(float DeltaSeconds) override;

  virtual FPathFollowingRequestResult MoveTo(
      const FAIMoveRequest& MoveRequest,
      FNavPathSharedPtr* OutPath = nullptr) override;

  virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult &Result) override;

  UFUNCTION(BlueprintCallable)
  void SenseActors(TArray<AActor *> Actors);

  EWalkerStatus GetWalkerStatus() const
  {
    return Status;
  }

private:

  void TryResumeMovement();

  void TryPauseMovement(bool bItWasRunOver = false);

  UFUNCTION()
  void OnPawnTookDamage(AActor *DamagedActor, float Damage, const UDamageType *DamageType, AController *InstigatedBy, AActor *DamageCauser);

  UPROPERTY(Category = "Walker AI Controller", VisibleAnywhere)
  UAISenseConfig_Sight *SightConfiguration;

  UPROPERTY(VisibleAnywhere)
  EWalkerStatus Status = EWalkerStatus::Unknown;
};
