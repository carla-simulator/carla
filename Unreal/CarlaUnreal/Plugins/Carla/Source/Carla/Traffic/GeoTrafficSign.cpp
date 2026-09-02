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
    return SpeedLimitStateForKmh(FCString::Atoi(*Subtype));
  }
  return ETrafficSignState::UNKNOWN;
}

void AGeoTrafficSign::PostInitializeComponents()
{
  Super::PostInitializeComponents();
  if (GetTrafficSignState() == ETrafficSignState::UNKNOWN && !XodrType.IsEmpty())
  {
    ConfigureForSignal(XodrType, XodrSubtype);
  }
}

void AGeoTrafficSign::ConfigureForSignal(const FString& Type, const FString& Subtype)
{
  XodrType = Type;
  XodrSubtype = Subtype;
  if (Type == TEXT("274"))
  {
    SetSpeedLimitKmh(FCString::Atoi(*Subtype));
  }
  else
  {
    SetTrafficSignState(StateForSignal(Type, Subtype));
  }
}
