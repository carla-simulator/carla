// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "DigitalTwinsTrafficLight.h"
#include "Carla.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ADigitalTwinsTrafficLight::ADigitalTwinsTrafficLight(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
}

void ADigitalTwinsTrafficLight::BeginPlay()
{
  Super::BeginPlay();

  UTrafficLightComponent* TLComponent = GetTrafficLightComponent();
  if (TLComponent)
  {
    TLComponent->LightChangeDispatcher.AddDynamic(this, &ADigitalTwinsTrafficLight::OnLightStateChanged);
    ETrafficLightState InitialState = TLComponent->GetLightState();
    UpdateDigitalTwinsMaterials(InitialState);
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("DigitalTwinsTrafficLight: No TrafficLightComponent found"));
  }
}

void ADigitalTwinsTrafficLight::OnLightStateChanged()
{
  UTrafficLightComponent* TLComponent = GetTrafficLightComponent();
  if (TLComponent)
  {
    ETrafficLightState NewState = TLComponent->GetLightState();
    UpdateDigitalTwinsMaterials(NewState);
  }
}

void ADigitalTwinsTrafficLight::UpdateDigitalTwinsMaterials(ETrafficLightState State)
{
  if (MaterialToLightType.Num() == 0)
  {
    TArray<UStaticMeshComponent*> MeshComponents;
    GetComponents<UStaticMeshComponent>(MeshComponents);

    struct FLightMaterialInfo
    {
      UMaterialInstanceDynamic* Material;
      FLinearColor Color;
      UStaticMeshComponent* MeshComp;
      int32 MatIdx;
    };
    TArray<FLightMaterialInfo> LightMaterials;

    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
      if (!MeshComp) continue;

      for (int32 MatIdx = 0; MatIdx < MeshComp->GetNumMaterials(); MatIdx++)
      {
        UMaterialInterface* Material = MeshComp->GetMaterial(MatIdx);
        if (!Material || !Material->GetName().Contains(TEXT("TrafficLight")))
          continue;

        UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(Material);
        if (!DynMaterial)
        {
          DynMaterial = UMaterialInstanceDynamic::Create(Material, MeshComp);
          if (DynMaterial)
          {
            MeshComp->SetMaterial(MatIdx, DynMaterial);
          }
        }

        if (DynMaterial)
        {
          FLinearColor EmissiveColor;
          if (DynMaterial->GetVectorParameterValue(FName("Emissive Color"), EmissiveColor))
          {
            FLightMaterialInfo Info;
            Info.Material = DynMaterial;
            Info.Color = EmissiveColor;
            Info.MeshComp = MeshComp;
            Info.MatIdx = MatIdx;
            LightMaterials.Add(Info);
          }
        }
      }
    }

    if (LightMaterials.Num() >= 3)
    {
      int32 GreenIdx = 0;
      for (int32 i = 1; i < LightMaterials.Num(); i++)
      {
        if (LightMaterials[i].Color.G > LightMaterials[GreenIdx].Color.G)
          GreenIdx = i;
      }

      int32 RedIdx = 0;
      for (int32 i = 1; i < LightMaterials.Num(); i++)
      {
        if (LightMaterials[i].Color.G < LightMaterials[RedIdx].Color.G)
          RedIdx = i;
      }

      int32 YellowIdx = -1;
      for (int32 i = 0; i < LightMaterials.Num(); i++)
      {
        if (i != GreenIdx && i != RedIdx)
        {
          YellowIdx = i;
          break;
        }
      }

      if (YellowIdx >= 0)
      {
        MaterialToLightType.Add(LightMaterials[GreenIdx].Material, ETrafficLightState::Green);
        MaterialToLightType.Add(LightMaterials[YellowIdx].Material, ETrafficLightState::Yellow);
        MaterialToLightType.Add(LightMaterials[RedIdx].Material, ETrafficLightState::Red);
      }
    }
    else
    {
      UE_LOG(LogCarla, Error, TEXT("Expected 3 traffic light materials but found %d"), LightMaterials.Num());
    }
  }

  for (const auto& Pair : MaterialToLightType)
  {
    UMaterialInstanceDynamic* DynMaterial = Pair.Key;
    ETrafficLightState LightType = Pair.Value;

    bool bShouldBeOn = (LightType == State);
    float IntensityValue = bShouldBeOn ? 50000.0f : 0.0f;

    DynMaterial->SetScalarParameterValue(FName("Emissive Intensity"), IntensityValue);
  }
}

FLinearColor ADigitalTwinsTrafficLight::GetColorForState(ETrafficLightState State) const
{
  switch (State)
  {
    case ETrafficLightState::Red:
      return FLinearColor(1.0f, 0.052f, 0.061f, 1.0f);

    case ETrafficLightState::Yellow:
      return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);

    case ETrafficLightState::Green:
      return FLinearColor(0.1f, 1.0f, 0.297f, 1.0f);

    default:
      return FLinearColor(0.05f, 0.05f, 0.05f, 1.0f);
  }
}
