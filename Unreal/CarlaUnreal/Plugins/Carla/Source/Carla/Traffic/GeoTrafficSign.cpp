// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "GeoTrafficSign.h"
#include "Carla.h"

AGeoTrafficSign::AGeoTrafficSign(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  Pole = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Pole"));
  Pole->SetupAttachment(RootComponent);
  Pole->SetMobility(EComponentMobility::Static);
  Pole->SetCollisionProfileName(TEXT("BlockAll"));

  Plate = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Plate"));
  Plate->SetupAttachment(RootComponent);
  Plate->SetMobility(EComponentMobility::Static);
  Plate->SetCollisionProfileName(TEXT("NoCollision"));
}

void AGeoTrafficSign::Setup(UStaticMesh* PoleMesh, UStaticMesh* PlateMesh, UMaterialInterface* PlateMaterial,
                            FVector PlateOffset, float PlateYaw, float PlateScale)
{
  Pole->SetStaticMesh(PoleMesh);
  Plate->SetStaticMesh(PlateMesh);
  if (PlateMaterial != nullptr)
  {
    Plate->SetMaterial(0, PlateMaterial);
  }
  Plate->SetRelativeLocation(PlateOffset);
  Plate->SetRelativeRotation(FRotator(0.0f, PlateYaw, 0.0f));
  Plate->SetRelativeScale3D(FVector(PlateScale));
}

ETrafficSignState AGeoTrafficSign::StateForSignal(const FString& Type, const FString& Subtype)
{
  if (Type == TEXT("206"))
  {
    return ETrafficSignState::StopSign;
  }
  if (Type == TEXT("205"))
  {
    return ETrafficSignState::YieldSign;
  }
  if (Type == TEXT("274"))
  {
    if (Subtype == TEXT("30")) return ETrafficSignState::SpeedLimit_30;
    if (Subtype == TEXT("40")) return ETrafficSignState::SpeedLimit_40;
    if (Subtype == TEXT("50")) return ETrafficSignState::SpeedLimit_50;
    if (Subtype == TEXT("60")) return ETrafficSignState::SpeedLimit_60;
    if (Subtype == TEXT("90")) return ETrafficSignState::SpeedLimit_90;
    if (Subtype == TEXT("100")) return ETrafficSignState::SpeedLimit_100;
    if (Subtype == TEXT("120")) return ETrafficSignState::SpeedLimit_120;
    if (Subtype == TEXT("130")) return ETrafficSignState::SpeedLimit_130;
  }
  return ETrafficSignState::UNKNOWN;
}
