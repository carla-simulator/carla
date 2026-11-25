// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "TrafficLightBase.h"
#include "Carla.h"
#include "Carla/Game/CarlaStatics.h"
#include "Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Vehicle/WheeledVehicleAIController.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

const FName ATrafficLightBase::ToolRigTag(TEXT("TrafficLightToolRig"));

// =============================================================================
// -- Traffic Light Tool lamps -------------------------------------------------
// =============================================================================

namespace
{
  // Every lamp of a Traffic Light Tool rig is one material slot named "led_<i>" on a module
  // mesh (ATrafficLightActor::AddModule / FMaterialFactory::CreateLightMaterialInstanceDynamic),
  // driven by a MID off M_TrafficLights_Inst with four parameters that survive the bake's
  // K2_CopyMaterialInstanceParameters: "Emissive Intensity", "Emissive Color" and the atlas
  // offsets "Offset U" / "Offset V".
  //
  // The atlas offset is the *glyph*, not the state. In the TrafficLights2025 "LightTypes"
  // DataTable, ArrowLeftGreen / ArrowLeftAmber / ArrowLeftRed all carry AtlasCoords (0, 1) and
  // differ only in Color, and the same holds for every other symbol -- so a UV table cannot
  // say which state a lamp belongs to. Colour can: the table uses exactly three of them.
  // The six symbols that carry no state at all (Tram, the four tram-line glyphs, Circle,
  // Triangle) are pure white and stay dark.
  const FLinearColor LampGreen(0.100902f, 1.000000f, 0.297537f, 1.0f);
  const FLinearColor LampAmber(0.930111f, 0.356400f, 0.014444f, 1.0f);
  const FLinearColor LampRed(1.000000f, 0.052026f, 0.061974f, 1.0f);

  // The three colours are far apart (nearest pair, amber<->red, is 0.41 apart in RGB), so a
  // generous radius still cannot confuse them and tolerates a slightly re-authored table.
  constexpr float LampColorToleranceSqr = 0.06f;

  // Emissive intensity of a lit lamp, the value the tool bakes.
  constexpr float LampOnIntensity = 50000.0f;

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
  /// ATrafficLightActor::Build via UniqueRename and preserved by the bake, so they survive
  /// into the level.
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

  void GetLampMeshComponents(const AActor& Actor, bool bOnlyToolRig, TArray<UStaticMeshComponent*>& Out)
  {
    Actor.GetComponents<UStaticMeshComponent>(Out);
    if (bOnlyToolRig)
    {
      Out.RemoveAll([](const UStaticMeshComponent* Comp)
      {
        return Comp == nullptr || !Comp->ComponentHasTag(ATrafficLightBase::ToolRigTag);
      });
    }
  }
}

// =============================================================================
// -- Static local methods -----------------------------------------------------
// =============================================================================

static bool IsValid(const ACarlaWheeledVehicle *Vehicle)
{
  return ((Vehicle != nullptr) && IsValidChecked(Vehicle));
}

static ETrafficSignState ToTrafficSignState(ETrafficLightState State)
{
  switch (State)
  {
    case ETrafficLightState::Green:
      return ETrafficSignState::TrafficLightGreen;
    case ETrafficLightState::Yellow:
      return ETrafficSignState::TrafficLightYellow;
    default:
    case ETrafficLightState::Red:
      return ETrafficSignState::TrafficLightRed;
  }
}

// =============================================================================
// -- ATrafficLightBase --------------------------------------------------------
// =============================================================================

ATrafficLightBase::ATrafficLightBase(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;
  TrafficLightComponent = CreateDefaultSubobject<UTrafficLightComponent>(TEXT("TrafficLightComponent"));
  if(TrafficLightComponent && RootComponent)
  {
    TrafficLightComponent->SetupAttachment(RootComponent);
  }
  SetTrafficSignState(ETrafficSignState::TrafficLightGreen);
}

ETrafficLightState ATrafficLightBase::GetTrafficLightState() const
{
  if (TrafficLightComponent)
  {
    return TrafficLightComponent->GetLightState();
  }
  else
  {
    return ETrafficLightState::Red;
  }
}

void ATrafficLightBase::SetTrafficLightState(const ETrafficLightState InState)
{
  if(TrafficLightComponent)
  {
    TrafficLightComponent->SetLightState(InState);
  }
  // OnTrafficLightStateChanged(State);
}

void ATrafficLightBase::NotifyWheeledVehicle(ACarlaWheeledVehicle *Vehicle)
{
  if (IsValid(Vehicle))
  {
    auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
    if (Controller != nullptr)
    {
      Controller->SetTrafficLightState(GetTrafficLightState());
      Vehicles.Add(Controller);
      Controller->SetTrafficLight(this);
    }
  }
}

void ATrafficLightBase::UnNotifyWheeledVehicle(ACarlaWheeledVehicle *Vehicle)
{
  if (IsValid(Vehicle))
  {
    auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
    if (Controller != nullptr)
    {
      Controller->SetTrafficLight(nullptr);
      Controller->SetTrafficLightState(ETrafficLightState::Green);
    }
  }
}

void ATrafficLightBase::SetGreenTime(float InGreenTime)
{
  if(TrafficLightComponent)
  {
    UTrafficLightController* TrafficLightController =
        TrafficLightComponent->GetController();
    check(TrafficLightController)
    TrafficLightController->SetGreenTime(InGreenTime);
    AddTimeToRecorder();
  }
}

float ATrafficLightBase::GetGreenTime() const
{
  if (TrafficLightComponent)
  {
    auto* Controller = TrafficLightComponent->GetController();
    check(Controller);
    return Controller->GetGreenTime();
  }
  else
  {
    return 0;
  }
}

void ATrafficLightBase::SetYellowTime(float InYellowTime)
{
  if(TrafficLightComponent)
  {
    UTrafficLightController* TrafficLightController =
      TrafficLightComponent->GetController();
    check(TrafficLightController)
    TrafficLightController->SetYellowTime(InYellowTime);
    AddTimeToRecorder();
  }
}

float ATrafficLightBase::GetYellowTime() const
{
  if (TrafficLightComponent)
  {
    auto* Controller = TrafficLightComponent->GetController();
    check(Controller);
    return Controller->GetYellowTime();
  }
  else
  {
    return 0;
  }
}

void ATrafficLightBase::SetRedTime(float InRedTime)
{
  if(TrafficLightComponent)
  {
    UTrafficLightController* TrafficLightController =
      TrafficLightComponent->GetController();
    check(TrafficLightController)
    TrafficLightController->SetRedTime(InRedTime);
    AddTimeToRecorder();
  }
}

float ATrafficLightBase::GetRedTime() const
{
  if (TrafficLightComponent)
  {
    auto* Controller = TrafficLightComponent->GetController();
    check(Controller);
    return Controller->GetRedTime();
  }
  else
  {
    return 0;
  }
}

float ATrafficLightBase::GetElapsedTime() const
{
  if (TrafficLightComponent)
  {
    auto* Controller = TrafficLightComponent->GetController();
    check(Controller);
    return Controller->GetElapsedTime();
  }
  else
  {
    return 0;
  }
}

void ATrafficLightBase::SetElapsedTime(float InElapsedTime)
{
  if (TrafficLightComponent)
  {
    auto* Controller = TrafficLightComponent->GetController();
    check(Controller);
    return Controller->SetElapsedTime(InElapsedTime);
  }
}

void ATrafficLightBase::SetTimeIsFrozen(bool InTimeIsFrozen)
{
  if(TrafficLightComponent)
  {
    TrafficLightComponent->SetFrozenGroup(InTimeIsFrozen);
  }
}

bool ATrafficLightBase::GetTimeIsFrozen() const
{
  if(TrafficLightComponent)
  {
    auto* Group = TrafficLightComponent->GetGroup();
    check(Group);
    return Group->IsFrozen();
  }
  return false;
}

void ATrafficLightBase::SetPoleIndex(int InPoleIndex)
{
  PoleIndex = InPoleIndex;
}

int ATrafficLightBase::GetPoleIndex() const
{
  return PoleIndex;
}

TArray<ATrafficLightBase *> ATrafficLightBase::GetGroupTrafficLights() const
{
  if(TrafficLightComponent)
  {
    TArray<ATrafficLightBase *> result;

    ATrafficLightGroup* Group = TrafficLightComponent->GetGroup();
    check(Group)

    for(auto& Controller : Group->GetControllers())
    {
      for(auto& TLComp : Controller->GetTrafficLights())
      {
        result.Add(Cast<ATrafficLightBase>(TLComp->GetOwner()));
      }
    }

    return result;
  }
  return GroupTrafficLights;
}

void ATrafficLightBase::SetGroupTrafficLights(TArray<ATrafficLightBase *> InGroupTrafficLights)
{
  GroupTrafficLights = InGroupTrafficLights;
}

UTrafficLightComponent* ATrafficLightBase::GetTrafficLightComponent()
{
  return TrafficLightComponent;
}
const UTrafficLightComponent* ATrafficLightBase::GetTrafficLightComponent() const
{
  return TrafficLightComponent;
}

void ATrafficLightBase::AddTimeToRecorder()
{
  auto * Recorder = UCarlaStatics::GetRecorder(GetWorld());
  if (Recorder && Recorder->IsEnabled())
  {
    Recorder->AddTrafficLightTime(*this);
  }
}

void ATrafficLightBase::LightChangedCompatibility(ETrafficLightState NewLightState)
{
  OnTrafficLightStateChanged(NewLightState);
  if (bUseToolRig)
  {
    // The blueprint's own state event may have just switched its lamps back on.
    HideBlueprintVisuals();
    UpdateToolRigLamps(NewLightState, true);
  }
}

void ATrafficLightBase::OnConstruction(const FTransform &Transform)
{
  Super::OnConstruction(Transform);
  if (bUseToolRig)
  {
    HideBlueprintVisuals();
  }
}

void ATrafficLightBase::BeginPlay()
{
  Super::BeginPlay();
  if (bUseToolRig)
  {
    HideBlueprintVisuals();
    UpdateToolRigLamps(GetTrafficLightState(), true);
  }
}

bool ATrafficLightBase::IsBlueprintTrafficLightPart(const UPrimitiveComponent* Component)
{
  const UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Component);
  if (MeshComp == nullptr || MeshComp->ComponentHasTag(ToolRigTag) || MeshComp->GetStaticMesh() == nullptr)
  {
    return false;
  }
  const FString Path = MeshComp->GetStaticMesh()->GetPathName();
  return Path.StartsWith(TEXT("/Game/Carla/Static/TrafficLight/")) ||
         Path.StartsWith(TEXT("/Game/Carla/Static/Pole/"));
}

void ATrafficLightBase::HideBlueprintVisuals()
{
  TArray<UStaticMeshComponent*> Components;
  GetComponents<UStaticMeshComponent>(Components);
  for (UStaticMeshComponent* Comp : Components)
  {
    if (!IsBlueprintTrafficLightPart(Comp))
    {
      continue;
    }
    // Hidden components are also skipped by the bounding boxes of the light.
    Comp->SetVisibility(false);
    Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  }
}

void ATrafficLightBase::RestoreBlueprintVisuals()
{
  TArray<UStaticMeshComponent*> Components;
  GetComponents<UStaticMeshComponent>(Components);
  for (UStaticMeshComponent* Comp : Components)
  {
    if (!IsBlueprintTrafficLightPart(Comp))
    {
      continue;
    }
    const UPrimitiveComponent* Template = Cast<UPrimitiveComponent>(Comp->GetArchetype());
    Comp->SetVisibility(Template ? Template->IsVisible() : true);
    Comp->SetCollisionEnabled(
        Template ? Template->GetCollisionEnabled() : ECollisionEnabled::QueryAndPhysics);
  }
}

void ATrafficLightBase::BuildLampMap(bool bOnlyToolRig)
{
  LampStates.Empty();
  PedestrianLamps.Empty();
  DarkLamps.Empty();

  TArray<UStaticMeshComponent*> MeshComponents;
  GetLampMeshComponents(*this, bOnlyToolRig, MeshComponents);
  LampMeshCount = MeshComponents.Num();

  struct FLamp
  {
    UMaterialInstanceDynamic* Material = nullptr;
    FString HeadKey;
    int32 ModuleIndex = 0;
    int32 SlotIndex = 0;
    bool bClassified = false;
    ETrafficLightState State = ETrafficLightState::Red;
    bool bStatelessGlyph = false;
    bool bPedestrian = false;
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
      // The tool's pedestrian heads are built from the crosswalk modules
      // (SM_TrafficLights_Crosswalk_*, SM_TrafficLights_Black_Crosswalk_*).
      Lamp.bPedestrian = MeshComp->GetStaticMesh()->GetName().Contains(TEXT("Crosswalk"));
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
  // up (Mod_00 SolidColorGreen, Mod_01 SolidColorAmber, Mod_02 SolidColorRed).
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
      DarkLamps.Add(Lamp.Material);
      (Lamp.bStatelessGlyph ? NumStateless : NumUnknown)++;
      continue;
    }
    LampStates.Add(Lamp.Material, Lamp.State);
    if (Lamp.bPedestrian)
    {
      PedestrianLamps.Add(Lamp.Material);
    }
    switch (Lamp.State)
    {
      case ETrafficLightState::Red:    ++NumRed; break;
      case ETrafficLightState::Yellow: ++NumYellow; break;
      default:                         ++NumGreen; break;
    }
  }

  // Nothing else drives these, and they are baked at "Emissive Intensity" 50000 (i.e. lit),
  // so an unclassified lamp must be switched off once or it stays permanently on.
  for (UMaterialInstanceDynamic* Dark : DarkLamps)
  {
    Dark->SetScalarParameterValue(FName("Emissive Intensity"), 0.0f);
  }

  const FString SignId = TrafficLightComponent ? TrafficLightComponent->GetSignId() : GetName();
  UE_LOG(LogCarla, Log,
      TEXT("Traffic light '%s' lamp census: %d mesh components, %d lamp slots -> "
           "%d red, %d yellow, %d green, %d of all those on pedestrian heads, %d stateless, %d unclassified (%d by module order)"),
      *SignId, LampMeshCount, Lamps.Num(),
      NumRed, NumYellow, NumGreen, PedestrianLamps.Num(), NumStateless, NumUnknown, NumFallback);

  if (Lamps.Num() == 0)
  {
    UE_LOG(LogCarla, Warning,
        TEXT("Traffic light '%s': no 'led_*' material slot on any of its %d mesh "
             "components; the rig will not animate"),
        *SignId, LampMeshCount);
  }
}

void ATrafficLightBase::UpdateToolRigLamps(ETrafficLightState State, bool bOnlyToolRig)
{
  // Rebuild when the actor has never been scanned, and again if it has grown components
  // since (UMapLogicParser re-parents a baked rig's meshes onto a digital twins light).
  TArray<UStaticMeshComponent*> MeshComponents;
  GetLampMeshComponents(*this, bOnlyToolRig, MeshComponents);
  if (LampMeshCount == INDEX_NONE || MeshComponents.Num() != LampMeshCount)
  {
    BuildLampMap(bOnlyToolRig);
  }

  for (const auto& Pair : LampStates)
  {
    UMaterialInstanceDynamic* DynMaterial = Pair.Key;
    if (DynMaterial == nullptr)
    {
      continue;
    }
    bool bLit = Pair.Value == State;
    if (PedestrianLamps.Contains(DynMaterial))
    {
      // No pedestrian phase: walk while the vehicles stop, don't walk otherwise.
      bLit = (Pair.Value == ETrafficLightState::Green && State == ETrafficLightState::Red) ||
             (Pair.Value == ETrafficLightState::Red && State != ETrafficLightState::Red);
    }
    DynMaterial->SetScalarParameterValue(FName("Emissive Intensity"), bLit ? LampOnIntensity : 0.0f);
  }
}
