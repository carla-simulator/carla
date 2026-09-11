// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "BlueprintLibrary/ParkedVehicleLibrary.h"

#include <util/ue-header-guard-begin.h>
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include <util/ue-header-guard-end.h>

bool UParkedVehicleLibrary::ConfigureParkedVehicleAppearance(
    AActor *Vehicle,
    UStaticMesh *Mesh,
    FLinearColor Color,
    UStaticMesh *GlassMesh,
    FName MeshComponentName,
    FName GlassComponentName,
    FString BodyworkSlotSubstring,
    FName ColorParameterName)
{
  if (!IsValid(Vehicle))
  {
    return false;
  }

  TArray<UStaticMeshComponent *> Components;
  Vehicle->GetComponents<UStaticMeshComponent>(Components);

  const auto FindComponent = [&Components](FName Name) -> UStaticMeshComponent *
  {
    for (UStaticMeshComponent *Component : Components)
    {
      if (Component->GetFName() == Name)
      {
        return Component;
      }
    }
    return nullptr;
  };

  UStaticMeshComponent *MeshComponent = FindComponent(MeshComponentName);
  if (MeshComponent == nullptr)
  {
    return false;
  }

  MeshComponent->SetStaticMesh(Mesh);

  // SetStaticMesh never clears whatever per-slot material overrides the
  // component already had (e.g. from a previous mesh assigned to this same
  // reused actor) -- without this, GetMaterial(MaterialIndex) below could
  // return a leftover override from a completely different vehicle instead
  // of the new mesh's own material, and the recolor would be built from
  // that stale source.
  MeshComponent->EmptyOverrideMaterials();

  // No two of these vehicles' meshes share one literal material slot name
  // ("MI_BodyWork_Mustang", "Vh_Car_AudiTT_BodyworkMat", ...), so this
  // looks for the first slot whose name merely contains
  // BodyworkSlotSubstring rather than requiring an exact match.
  int32 MaterialIndex = INDEX_NONE;
  if (Mesh != nullptr)
  {
    const TArray<FStaticMaterial> &StaticMaterials = Mesh->GetStaticMaterials();
    for (int32 Index = 0; Index < StaticMaterials.Num(); ++Index)
    {
      if (StaticMaterials[Index].MaterialSlotName.ToString().Contains(BodyworkSlotSubstring, ESearchCase::IgnoreCase))
      {
        MaterialIndex = Index;
        break;
      }
    }
  }
  if (MaterialIndex != INDEX_NONE)
  {
    // Always a per-instance dynamic material instance -- never write the
    // color onto the shared asset itself, which would repaint every other
    // vehicle using it too.
    UMaterialInterface *SourceMaterial = MeshComponent->GetMaterial(MaterialIndex);
    UMaterialInstanceDynamic *DynamicMaterial = UMaterialInstanceDynamic::Create(SourceMaterial, MeshComponent);
    DynamicMaterial->SetVectorParameterValue(ColorParameterName, Color);
    MeshComponent->SetMaterial(MaterialIndex, DynamicMaterial);
  }

  if (GlassMesh != nullptr)
  {
    if (UStaticMeshComponent *GlassComponent = FindComponent(GlassComponentName))
    {
      GlassComponent->SetStaticMesh(GlassMesh);
      GlassComponent->EmptyOverrideMaterials();
    }
  }

  if (USceneComponent *Root = Vehicle->GetRootComponent())
  {
    Root->SetMobility(EComponentMobility::Static);
  }

  return true;
}
