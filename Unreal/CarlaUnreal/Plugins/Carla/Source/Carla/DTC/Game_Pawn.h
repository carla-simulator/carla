#pragma once

#include "GameFramework/Pawn.h"

#include "Game_Pawn.generated.h"

UCLASS()
class CARLA_API AGame_Pawn : public APawn
{
	GENERATED_BODY()

	// ---------------------------------
	// --- Constructors
	// ---------------------------------
public:
	AGame_Pawn(const FObjectInitializer& objectInitializer);

};