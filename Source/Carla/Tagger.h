// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/Actor.h"
#include "Tagger.generated.h"

UCLASS()
class CARLA_API ATagger : public AActor
{
  GENERATED_BODY()

public:

  ATagger();

protected:

#if WITH_EDITOR
  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

private:

  void TagObjects();

  UPROPERTY(Category = "Tagger", EditAnywhere)
  bool bTriggerTagObjects = false;
};
