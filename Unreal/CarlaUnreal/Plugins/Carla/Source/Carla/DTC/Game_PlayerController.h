#pragma once

#include "GameFramework/PlayerController.h"

#include "Game_PlayerController.generated.h"

UCLASS()
class CARLA_API AGame_PlayerController : public APlayerController
{
	GENERATED_BODY()

		// ---------------------------------
		// --- Constructors
		// ---------------------------------
public:
	AGame_PlayerController(const FObjectInitializer& objectInitializer);

	UFUNCTION(Exec)
	void SetFixedTimestep(double seconds);
};