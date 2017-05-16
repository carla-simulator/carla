// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "AIController.h"
#include "WalkerAIController.generated.h"

UCLASS()
class CARLA_API AWalkerAIController : public AAIController
{
  GENERATED_BODY()

public:

  AWalkerAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
