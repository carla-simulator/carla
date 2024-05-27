#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Game_GameMode.generated.h"

/**
 * 
 */
UCLASS()
class CARLA_API AGame_GameMode : public AGameModeBase
{
	GENERATED_BODY()

	// ---------------------------------
	// --- Constructors
	// ---------------------------------
public:
	AGame_GameMode(const FObjectInitializer& objectInitializer);
	
};
