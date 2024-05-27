#include "Game_GI_DARPA.h"
#include "Kismet/GameplayStatics.h"

UGame_GI_DARPA::UGame_GI_DARPA()
{
	IsSimulationReady = false;
	MaxSimulationTime = 900.;
	bQuitOnVehiclePathEnd = true;
}

void UGame_GI_DARPA::Init()
{
	Super::Init();

}

// Carla Functions (implemented in blueprint)
void UGame_GI_DARPA::CarlaPrepareSimulation_Implementation(int32 PathIndex)
{
	
}

bool UGame_GI_DARPA::CarlaIsReady_Implementation()
{
	return IsSimulationReady;
}

float UGame_GI_DARPA::CarlaGetSimulationTime_Implementation()
{
	if(SimulationTimeHandle.IsValid())
	{
		return TimerManager->GetTimerElapsed(SimulationTimeHandle);
	}
	return 0;
}

bool UGame_GI_DARPA::CarlaStartSimulation_Implementation()
{
	return false;
}

bool UGame_GI_DARPA::CarlaResumeVehicle_Implementation(FName VehicleID)
{
	return false;
}

bool UGame_GI_DARPA::CarlaPauseVehicle_Implementation(FName VehicleID)
{
	return false;
}

bool UGame_GI_DARPA::CarlaNextWaypoint_Implementation(FName VehicleID)
{
	return false;
}

void UGame_GI_DARPA::CarlaStopUnreal()
{
	UE_LOG(LogTemp,	Warning, TEXT("UGame_GI_DARPA::CarlaStopUnreal() triggered, ending Unreal..."));
	UKismetSystemLibrary::QuitGame(GetWorld(),UGameplayStatics::GetPlayerController(GetWorld(),0),EQuitPreference::Type::Quit,false);
}

void UGame_GI_DARPA::ParseYamlStruct()
{
	
}
void UGame_GI_DARPA::SetIREnabled_Implementation(const bool newEnabled)
{

}

void UGame_GI_DARPA::StartSimulationTimer()
{
	GetTimerManager().SetTimer(SimulationTimeHandle, this, &UGame_GI_DARPA::HandleSimulationTimerComplete, MaxSimulationTime, false);
}

void UGame_GI_DARPA::HandleSimulationTimerComplete()
{
	CarlaStopUnreal();
}

void UGame_GI_DARPA::HandleVehiclePathComplete(const FName& VehicleID)
{
	if (bQuitOnVehiclePathEnd)
	{
		CarlaStopUnreal();
	}
}

UGame_GI_DARPA* UGame_GI_DARPA::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return World ? Cast<UGame_GI_DARPA>(World->GetGameInstance()) : nullptr;
}

