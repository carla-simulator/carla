#include "Game_PlayerController.h"

AGame_PlayerController::AGame_PlayerController(const FObjectInitializer& objectInitializer) :
	Super(objectInitializer)
{

}

void AGame_PlayerController::SetFixedTimestep(double seconds)
{
	FApp::SetFixedDeltaTime(seconds);
	FApp::SetUseFixedTimeStep(true);
}
