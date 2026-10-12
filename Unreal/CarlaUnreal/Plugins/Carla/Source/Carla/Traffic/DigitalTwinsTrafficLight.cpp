// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "DigitalTwinsTrafficLight.h"
#include "Carla.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
  // ---------------------------------------------------------------- lamp classification
  //
  // Every lamp of a Traffic Light Tool rig is one material slot named "led_<i>" on a module
  // mesh (ATrafficLightActor::AddModule / FMaterialFactory::CreateLightMaterialInstanceDynamic),
  // driven by a MID off M_TrafficLights_Inst with four parameters that survive Bake()'s
  // K2_CopyMaterialInstanceParameters: "Emissive Intensity", "Emissive Color" and the atlas
  // offsets "Offset U" / "Offset V".
  //
  // The atlas offset is the *glyph*, not the state. In the TrafficLights2025 "LightTypes"
  // DataTable, ArrowLeftGreen / ArrowLeftAmber / ArrowLeftRed all carry AtlasCoords (0, 1) and
  // differ only in Color, and the same holds for every other symbol -- so a UV table cannot
  // say which state a lamp belongs to. Colour can: the table uses exactly three of them.
  // The atlas offset is still useful for the six symbols that carry no state at all (Tram,
  // the four tram-line glyphs, Circle, Triangle): those are pure white and stay dark.
  const FLinearColor LampGreen(0.100902f, 1.000000f, 0.297537f, 1.0f);
  const FLinearColor LampAmber(0.930111f, 0.356400f, 0.014444f, 1.0f);
  const FLinearColor LampRed(1.000000f, 0.052026f, 0.061974f, 1.0f);

  // The three colours are far apart (nearest pair, amber<->red, is 0.41 apart in RGB), so a
  // generous radius still cannot confuse them and tolerates a slightly re-authored table.
  constexpr float LampColorToleranceSqr = 0.06f;

  float ColorDistSqr(const FLinearColor& A, const FLinearColor& B)
  {
    return FMath::Square(A.R - B.R) + FMath::Square(A.G - B.G) + FMath::Square(A.B - B.B);
  }

  bool ClassifyLampByColor(const FLinearColor& Color, ETrafficLightState& OutState)
  {
    const float DGreen = ColorDistSqr(Color, LampGreen);
    const float DAmber = ColorDistSqr(Color, LampAmber);
    const float DRed   = ColorDistSqr(Color, LampRed);
    const float Best = FMath::Min3(DGreen, DAmber, DRed);
    if (Best > LampColorToleranceSqr)
    {
      return false;  // white (stateless glyph) or a material whose colour was never set
    }
    OutState = (Best == DGreen) ? ETrafficLightState::Green
             : (Best == DAmber) ? ETrafficLightState::Yellow
                                : ETrafficLightState::Red;
    return true;
  }

  /// "Pole_00_Head_01_Mod_02" -> ("Pole_00_Head_01", 2). Component names are assigned by
  /// ATrafficLightActor::Build via UniqueRename and preserved by Bake
  /// (BakedMeshComponent->Rename(*Source->GetName(), ...)), so they survive into the level.
  bool SplitHeadAndModule(const FString& ComponentName, FString& OutHeadKey, int32& OutModuleIndex)
  {
    int32 At = INDEX_NONE;
    if (!ComponentName.FindLastChar(TEXT('_'), At))
    {
      return false;
    }
    const FString Tail = ComponentName.Mid(At + 1);
    if (Tail.IsEmpty() || !Tail.IsNumeric())
    {
      return false;
    }
    const FString Head = ComponentName.Left(At);
    if (!Head.EndsWith(TEXT("_Mod")))
    {
      return false;
    }
    OutHeadKey = Head.LeftChop(4);  // drop "_Mod"
    OutModuleIndex = FCString::Atoi(*Tail);
    return true;
  }
}

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

void ADigitalTwinsTrafficLight::BuildLampMap()
{
  MaterialToLightType.Empty();
  DarkMaterials.Empty();

  TArray<UStaticMeshComponent*> MeshComponents;
  GetComponents<UStaticMeshComponent>(MeshComponents);
  CachedMeshComponentCount = MeshComponents.Num();

  struct FLamp
  {
    UMaterialInstanceDynamic* Material = nullptr;
    FString HeadKey;
    int32 ModuleIndex = 0;
    int32 SlotIndex = 0;
    bool bClassified = false;
    ETrafficLightState State = ETrafficLightState::Red;
    bool bStatelessGlyph = false;
  };
  TArray<FLamp> Lamps;

  for (UStaticMeshComponent* MeshComp : MeshComponents)
  {
    if (!MeshComp || !MeshComp->GetStaticMesh())
    {
      continue;
    }

    // A lamp is identified by its *material slot* name, not by the material's name: the
    // material on a baked rig is an auto-named MID ("MID_M_TrafficLights_Inst_7"), which only
    // happens to contain "TrafficLight".
    const TArray<FName> SlotNames = MeshComp->GetMaterialSlotNames();
    for (int32 Slot = 0; Slot < SlotNames.Num(); ++Slot)
    {
      if (!SlotNames[Slot].ToString().StartsWith(TEXT("led_")))
      {
        continue;
      }
      const int32 MatIdx = MeshComp->GetMaterialIndex(SlotNames[Slot]);
      if (MatIdx == INDEX_NONE)
      {
        continue;
      }
      UMaterialInterface* Material = MeshComp->GetMaterial(MatIdx);
      if (!Material)
      {
        continue;
      }
      UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(Material);
      if (!DynMaterial)
      {
        DynMaterial = UMaterialInstanceDynamic::Create(Material, MeshComp);
        if (!DynMaterial)
        {
          continue;
        }
        MeshComp->SetMaterial(MatIdx, DynMaterial);
      }

      FLamp Lamp;
      Lamp.Material = DynMaterial;
      Lamp.SlotIndex = Slot;
      if (!SplitHeadAndModule(MeshComp->GetName(), Lamp.HeadKey, Lamp.ModuleIndex))
      {
        Lamp.HeadKey = MeshComp->GetName();
        Lamp.ModuleIndex = 0;
      }

      FLinearColor EmissiveColor;
      if (DynMaterial->GetVectorParameterValue(FName("Emissive Color"), EmissiveColor))
      {
        Lamp.bClassified = ClassifyLampByColor(EmissiveColor, Lamp.State);
        if (!Lamp.bClassified)
        {
          // A pure-white lamp is one of the stateless glyphs (Tram / tram lines / Circle /
          // Triangle); anything else is a lamp whose colour never made it through the bake.
          Lamp.bStatelessGlyph = EmissiveColor.R > 0.9f && EmissiveColor.G > 0.9f && EmissiveColor.B > 0.9f;
        }
      }
      Lamps.Add(Lamp);
    }
  }

  // Fallback for a head whose colours did not survive: order its lamps by module index and
  // read them as a plain 3-aspect head. The Traffic Light Tool builds a vertical head bottom
  // up and every rig in ue/rigs follows the same order (see ue/rigs/eu_pole.json:
  // Mod_00 SolidColorGreen, Mod_01 SolidColorAmber, Mod_02 SolidColorRed).
  static const ETrafficLightState ThreeAspect[3] = {
      ETrafficLightState::Green, ETrafficLightState::Yellow, ETrafficLightState::Red};
  int32 NumFallback = 0;
  TMap<FString, TArray<int32>> ByHead;
  for (int32 i = 0; i < Lamps.Num(); ++i)
  {
    ByHead.FindOrAdd(Lamps[i].HeadKey).Add(i);
  }
  for (auto& HeadPair : ByHead)
  {
    TArray<int32>& Indices = HeadPair.Value;
    if (Indices.ContainsByPredicate([&](int32 i) { return Lamps[i].bClassified || Lamps[i].bStatelessGlyph; }))
    {
      continue;  // this head classified fine (or is deliberately dark)
    }
    if (Indices.Num() != 3)
    {
      continue;
    }
    Indices.Sort([&](int32 A, int32 B)
    {
      return Lamps[A].ModuleIndex != Lamps[B].ModuleIndex
                 ? Lamps[A].ModuleIndex < Lamps[B].ModuleIndex
                 : Lamps[A].SlotIndex < Lamps[B].SlotIndex;
    });
    for (int32 k = 0; k < 3; ++k)
    {
      Lamps[Indices[k]].bClassified = true;
      Lamps[Indices[k]].State = ThreeAspect[k];
      ++NumFallback;
    }
  }

  int32 NumRed = 0, NumYellow = 0, NumGreen = 0, NumStateless = 0, NumUnknown = 0;
  for (const FLamp& Lamp : Lamps)
  {
    if (!Lamp.bClassified)
    {
      DarkMaterials.Add(Lamp.Material);
      (Lamp.bStatelessGlyph ? NumStateless : NumUnknown)++;
      continue;
    }
    MaterialToLightType.Add(Lamp.Material, Lamp.State);
    switch (Lamp.State)
    {
      case ETrafficLightState::Red:    ++NumRed; break;
      case ETrafficLightState::Yellow: ++NumYellow; break;
      default:                         ++NumGreen; break;
    }
  }

  // Nothing else drives these, and they are baked at "Emissive Intensity" 50000 (i.e. lit),
  // so an unclassified lamp must be switched off once or it stays permanently on.
  for (UMaterialInstanceDynamic* Dark : DarkMaterials)
  {
    Dark->SetScalarParameterValue(FName("Emissive Intensity"), 0.0f);
  }

  UE_LOG(LogCarla, Log,
      TEXT("DigitalTwinsTrafficLight '%s' lamp census: %d mesh components, %d lamp slots -> "
           "%d red, %d yellow, %d green, %d stateless, %d unclassified (%d by module order)"),
      *GetTrafficLightComponent()->GetSignId(), CachedMeshComponentCount, Lamps.Num(),
      NumRed, NumYellow, NumGreen, NumStateless, NumUnknown, NumFallback);

  if (Lamps.Num() == 0)
  {
    UE_LOG(LogCarla, Warning,
        TEXT("DigitalTwinsTrafficLight '%s': no 'led_*' material slot on any of its %d mesh "
             "components; the rig will not animate"),
        *GetTrafficLightComponent()->GetSignId(), CachedMeshComponentCount);
  }
}

void ADigitalTwinsTrafficLight::UpdateDigitalTwinsMaterials(ETrafficLightState State)
{
  // Rebuild when the actor has never been scanned, and again if it has grown components
  // since (UMapLogicParser re-parents the baked rig's meshes onto this actor).
  TArray<UStaticMeshComponent*> MeshComponents;
  GetComponents<UStaticMeshComponent>(MeshComponents);
  if (MaterialToLightType.Num() == 0 || MeshComponents.Num() != CachedMeshComponentCount)
  {
    BuildLampMap();
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
      return LampRed;

    case ETrafficLightState::Yellow:
      return LampAmber;

    case ETrafficLightState::Green:
      return LampGreen;

    default:
      return FLinearColor(0.05f, 0.05f, 0.05f, 1.0f);
  }
}
