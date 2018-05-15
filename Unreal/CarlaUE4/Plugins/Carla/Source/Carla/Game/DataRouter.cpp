// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "DataRouter.h"

#include "Sensor/Sensor.h"
#include "CarlaPlayerState.h"

void FDataRouter::RegisterSensor(ASensor &InSensor)
{
  if (SensorDataSink.IsValid()) {
    InSensor.SetSensorDataSink(SensorDataSink);
  } else {
    UE_LOG(
        LogCarla,
        Error,
        TEXT("FDataRouter: Trying to register a sensor but I don't have a SensorDataSink"));
  }
}

void FDataRouter::RestartLevel()
{
  if (Player != nullptr) {
    Player->RestartLevel();
    Player = nullptr;
  } else {
    UE_LOG(
        LogCarla,
        Error,
        TEXT("FDataRouter: Trying to restart level but I don't have any player registered"));
  }
}

bool FDataRouter::PlayerControlVehicle(APlayerController* playercontroller, APawn* pawn)
{
  if (!IsValid(playercontroller) || playercontroller->IsPendingKill() || !IsValid(pawn) || pawn->IsPendingKill()) return false;
  AController* oldcontroller = pawn->GetController();
  if (IsValid(oldcontroller) && !oldcontroller->IsA<AWheeledVehicleController>())
  {
      oldcontroller->UnPossess();
      oldcontroller->SetPawn(nullptr);
      oldcontroller->Destroy();
  }

  //check player controller and pawn
  if (!pawn->IsA<ACarlaWheeledVehicle>()) {
      playercontroller->Possess(pawn);
      return false;
  }
  ACarlaVehicleController* carlavehiclecontroller = Cast<ACarlaVehicleController>(playercontroller);
  if(carlavehiclecontroller && carlavehiclecontroller->IsPossessingThePlayer())
  {
    ACarlaWheeledVehicle* vehicle = Cast<ACarlaWheeledVehicle>(pawn);
    vehicle->Controller = carlavehiclecontroller;
    vehicle->AIControllerClass = ACarlaVehicleController::StaticClass();
    carlavehiclecontroller->SetAutopilot(false);
    carlavehiclecontroller->SetupInputComponent();
    carlavehiclecontroller->StopSpectatingOnly();

  }
    
  playercontroller->Possess(pawn);
  
  return true;
}
