#include "Carla/DTC/MessageFactory.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

// Set Variables
AMessageFactory::AMessageFactory()
{
	IsSimulationReady = false;
	MaxSimulationTime = 900.;
	bQuitOnVehiclePathEnd = true;
}

// BeginPlay
void AMessageFactory::BeginPlay()
{
	Super::BeginPlay();
}

// Start Simulation
void AMessageFactory::UPy_StartSimulation_Implementation()
{
}

// Stop Simulation
void AMessageFactory::UPy_StopSimulation_Implementation()
{
}

// Ready checks
bool AMessageFactory::GetIsSimulationReady()
{
	return IsSimulationReady;
}

// Stop Unreal
void AMessageFactory::CarlaStopUnreal()
{
	UE_LOG(LogTemp,	Warning, TEXT("UGame_GI_DARPA::CarlaStopUnreal() triggered, ending Unreal..."));
	UKismetSystemLibrary::QuitGame(GetWorld(),UGameplayStatics::GetPlayerController(GetWorld(),0),EQuitPreference::Type::Quit,false);
}

// Timer - SetLength
void AMessageFactory::UPy_SetTimerLength(float TimerLength)
{
	MaxSimulationTime = TimerLength;
}

// Timer - Start
void AMessageFactory::StartSimulationTimer()
{
	if (GetWorld())
	{
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		TimerManager.SetTimer(SimulationTimeHandle, this, &AMessageFactory::HandleSimulationTimerComplete, MaxSimulationTime, false);
	}
}

// Timer - Get Time Remaining
float AMessageFactory::CarlaGetSimulationTime()
{
	if (GetWorld() && SimulationTimeHandle.IsValid())
	{
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		return TimerManager.GetTimerRemaining(SimulationTimeHandle);
	}
	return 0;
}

// Timer - Stop Unreal
void AMessageFactory::HandleSimulationTimerComplete()
{
	CarlaStopUnreal();
}

// Vehicle Stop Unreal
void AMessageFactory::HandleVehiclePathComplete(const FName& VehicleID)
{
	if (bQuitOnVehiclePathEnd)
	{
		CarlaStopUnreal();
	}
}