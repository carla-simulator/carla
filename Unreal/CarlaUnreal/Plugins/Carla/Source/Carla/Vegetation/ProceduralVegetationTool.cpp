#include "ProceduralVegetationTool.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Misc/PackageName.h"
#include "Data/PCGPointData.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Game/Tagger.h"
#include "Metadata/PCGMetadata.h"
#include "Metadata/PCGMetadataAttributeTpl.h"
#include "Misc/FileHelper.h"
#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGGraph.h"
#include "PCGSubsystem.h"
#include "Async/TaskGraphInterfaces.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#if WITH_EDITOR
#include "StaticMeshCompiler.h"
#endif

AProceduralVegetationTool::AProceduralVegetationTool()
{
  RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
  RootComponent->SetMobility(EComponentMobility::Static);
  GenerationBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("GenerationBounds"));
  GenerationBounds->SetupAttachment(RootComponent);
  GenerationBounds->SetBoxExtent(FVector(100));
  GenerationBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  GenerationBounds->SetHiddenInGame(true);
  GenerationBounds->SetCanEverAffectNavigation(false);
  PCG = CreateDefaultSubobject<UPCGComponent>(TEXT("VegetationPCG"));
  PCG->GenerationTrigger = EPCGComponentGenerationTrigger::GenerateOnDemand;
  PCG->OnPCGGraphGeneratedExternal.AddDynamic(this, &AProceduralVegetationTool::OnGenerated);
  Graph = TSoftObjectPtr<UPCGGraphInterface>(FSoftObjectPath(TEXT("/Game/Carla/Blueprints/PCG/VegetationTool/PCG_TwinVegetation.PCG_TwinVegetation")));
  PrimaryActorTick.bCanEverTick = false;
#if WITH_EDITORONLY_DATA
  bIsSpatiallyLoaded = false;
#endif
}

bool AProceduralVegetationTool::ImportPlan()
{
  if (IsGenerating()) { Status = TEXT("Wait for PCG generation to finish."); return false; }
  FString Text;
  TSharedPtr<FJsonObject> Root;
  if (!FFileHelper::LoadFileToString(Text, *PlanFile.FilePath) ||
      !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root) || !Root.IsValid())
  { Status = TEXT("Cannot read vegetation plan JSON."); return false; }
  FString Schema, Coordinates;
  const TArray<TSharedPtr<FJsonValue>>* Entries = nullptr;
  if (!Root->TryGetStringField(TEXT("schema"), Schema) || Schema != TEXT("twin-vegetation-plan/1") ||
      !Root->TryGetStringField(TEXT("coordinate_system"), Coordinates) || Coordinates != TEXT("carla-metres") ||
      !Root->TryGetArrayField(TEXT("points"), Entries) || Entries->Num() > 200000)
  { Status = TEXT("Unsupported plan schema, coordinates or point count."); return false; }
  TArray<FCarlaVegetationPoint> Candidate;
  TSet<FString> Ids;
#if WITH_EDITOR
  // Spatial ground actors can have just been loaded by the commandlet. Their
  // asynchronous mesh compilation must finish before collision bodies exist.
  FStaticMeshCompilingManager::Get().FinishAllCompilation();
#endif
  FCollisionQueryParams Query(SCENE_QUERY_STAT(VegetationGround), true, this);
  for (TActorIterator<AProceduralVegetationTool> It(GetWorld()); It; ++It) Query.AddIgnoredActor(*It);
  for (const auto& Entry : *Entries)
  {
    const TSharedPtr<FJsonObject>* ObjPtr = nullptr;
    if (!Entry->TryGetObject(ObjPtr)) { Status = TEXT("Invalid point object."); return false; }
    const auto& Obj = *ObjPtr;
    FCarlaVegetationPoint Point;
    FString MeshPath, State, Kind;
    double X, Y, Z, Scale, Yaw, Seed, PitRadius, CrownRadius;
    if (!Obj->TryGetStringField(TEXT("id"), Point.Id) || Point.Id.IsEmpty() || Ids.Contains(Point.Id) ||
        !Obj->TryGetStringField(TEXT("status"), State) || State != TEXT("accepted") ||
        !Obj->TryGetStringField(TEXT("mesh"), MeshPath) || !MeshPath.StartsWith(TEXT("/Game/Carla/Static/Vegetation/")) ||
        !Obj->TryGetStringField(TEXT("source"), Point.Provenance) ||
        !Obj->TryGetStringField(TEXT("kind"), Kind) ||
        !Obj->TryGetNumberField(TEXT("pit_radius_m"), PitRadius) ||
        !Obj->TryGetNumberField(TEXT("crown_radius_m"), CrownRadius) ||
        !FMath::IsFinite(PitRadius) || !FMath::IsFinite(CrownRadius) || PitRadius <= 0 || CrownRadius <= 0 ||
        !Obj->TryGetNumberField(TEXT("x"), X) || !Obj->TryGetNumberField(TEXT("y"), Y) ||
        !Obj->TryGetNumberField(TEXT("z"), Z) || !Obj->TryGetNumberField(TEXT("scale"), Scale) ||
        !Obj->TryGetNumberField(TEXT("yaw"), Yaw) || !Obj->TryGetNumberField(TEXT("seed"), Seed) ||
        !FMath::IsFinite(X) || !FMath::IsFinite(Y) || !FMath::IsFinite(Z) ||
        !FMath::IsFinite(Scale) || Scale <= 0 || Scale > 10 || !FMath::IsFinite(Yaw) ||
        !FMath::IsFinite(Seed) || Seed < 0 || Seed > MAX_int32)
    { Status = TEXT("Invalid or duplicate vegetation point; previous plan retained."); return false; }
    if (!MeshPath.Contains(TEXT("."))) MeshPath += TEXT(".") + FPackageName::GetShortName(MeshPath);
    Point.Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(MeshPath));
    if (!Point.Mesh.LoadSynchronous())
    { Status = FString::Printf(TEXT("Missing mesh: %s; previous plan retained."), *MeshPath); return false; }
    const FBoxSphereBounds MeshBounds = Point.Mesh.Get()->GetBounds();
    const double Radius = (FVector2D(MeshBounds.BoxExtent).Size() + FVector2D(MeshBounds.Origin).Size()) * Scale / 100.0;
    if ((Kind == TEXT("tree") ? CrownRadius : PitRadius) + 0.01 < Radius)
    { Status = FString::Printf(TEXT("Mesh footprint exceeds planned clearance for %s (%.2f m). Update the preset and regenerate."), *Point.Id, Radius); return false; }
    // Project only onto an explicit planting surface, never onto arbitrary WorldStatic
    // (roofs, road traffic, or other generated vegetation). A missing/mismatched surface
    // invalidates the import before any existing bake is replaced.
    FVector Position = FVector(X,Y,Z)*100.0;
    FHitResult Hit;
    if (!GetWorld()->LineTraceSingleByChannel(Hit, Position+FVector(0,0,100), Position-FVector(0,0,200), ECC_Visibility, Query))
    { Status = FString::Printf(TEXT("No planting surface beneath %s; load the region before importing."), *Point.Id); return false; }
    const UStaticMeshComponent* Ground = Cast<UStaticMeshComponent>(Hit.GetComponent());
    const FString GroundPath = Ground && Ground->GetStaticMesh() ? Ground->GetStaticMesh()->GetPathName() : FString();
    if (!GroundPath.StartsWith(TEXT("/Game/Carla/Static/SideWalk/")) && !GroundPath.StartsWith(TEXT("/Game/Carla/Static/Terrain/")))
    { Status = FString::Printf(TEXT("Unexpected planting surface beneath %s: %s"), *Point.Id, *GroundPath); return false; }
    Position.Z = Hit.ImpactPoint.Z;
    TOptional<FCarlaVegetationPoint> Footing;
    const TSharedPtr<FJsonObject>* FootingJson = nullptr;
    if (Obj->TryGetObjectField(TEXT("footing"), FootingJson))
    {
      FString Path;
      double FootScale, FootRadius, Offset, SoilHeight, FootYaw;
      const auto& F = *FootingJson;
      if (Kind != TEXT("tree") || !F->TryGetStringField(TEXT("mesh"), Path) ||
          Path != TEXT("/Game/Carla/Static/Static/SM_TreeBase02") ||
          !F->TryGetNumberField(TEXT("scale"), FootScale) || !FMath::IsFinite(FootScale) || FootScale <= 0 || FootScale > 3 ||
          !F->TryGetNumberField(TEXT("radius_m"), FootRadius) || !FMath::IsFinite(FootRadius) || FootRadius <= 0 ||
          !F->TryGetNumberField(TEXT("ground_offset_m"), Offset) || !FMath::IsFinite(Offset) || Offset < -0.04 || Offset > -0.005 ||
          !F->TryGetNumberField(TEXT("soil_height_m"), SoilHeight) || !FMath::IsFinite(SoilHeight) || FMath::Abs(SoilHeight-0.14744362) > 0.0001 ||
          !F->TryGetNumberField(TEXT("yaw"), FootYaw) || !FMath::IsFinite(FootYaw) ||
          !GroundPath.StartsWith(TEXT("/Game/Carla/Static/SideWalk/")))
      { Status = FString::Printf(TEXT("Invalid sidewalk footing for %s."), *Point.Id); return false; }
      FCarlaVegetationPoint Base;
      Base.Id = Point.Id + TEXT(":footing");
      Base.Provenance = Point.Provenance + TEXT(":footing");
      Path += TEXT(".") + FPackageName::GetShortName(Path);
      Base.Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(Path));
      if (!Base.Mesh.LoadSynchronous()) { Status = TEXT("Missing tree footing mesh."); return false; }
      const auto B = Base.Mesh.Get()->GetBounds();
      const double RequiredRadius = (FVector2D(B.BoxExtent).Size()+FVector2D(B.Origin).Size())*FootScale/100.0;
      if (PitRadius+0.001 < RequiredRadius || FootRadius*FootScale+0.001 < RequiredRadius)
      { Status = FString::Printf(TEXT("Footing exceeds reserved pit for %s."), *Point.Id); return false; }
      // Embed the raised stone surround in the sidewalk plane. Its soil surface
      // covers the paving without coplanar flicker; seat the upright trunk on
      // that soil. The measured center height belongs to this specific asset.
      const FQuat Rotation = FRotationMatrix::MakeFromZX(Hit.ImpactNormal, FRotator(0, FootYaw, 0).Vector()).ToQuat();
      const double EdgeRadius = FMath::Max(B.BoxExtent.X, B.BoxExtent.Y)*FootScale;
      for (int32 I=0; I<8; ++I)
      {
        const double A = I*PI/4;
        const FVector Edge = Position + Rotation.RotateVector(FVector(FMath::Cos(A)*EdgeRadius, FMath::Sin(A)*EdgeRadius, 0));
        FHitResult EdgeHit;
        const bool bHit = GetWorld()->LineTraceSingleByChannel(EdgeHit, Edge+FVector(0,0,50), Edge-FVector(0,0,50), ECC_Visibility, Query);
        const auto* EdgeMesh = bHit ? Cast<UStaticMeshComponent>(EdgeHit.GetComponent()) : nullptr;
        if (!EdgeMesh || !EdgeMesh->GetStaticMesh() ||
            !EdgeMesh->GetStaticMesh()->GetPathName().StartsWith(TEXT("/Game/Carla/Static/SideWalk/")) ||
            FMath::Abs(EdgeHit.ImpactPoint.Z-Edge.Z) > 1.0)
        { Status = FString::Printf(TEXT("Footing crosses a sidewalk edge or uneven surface at %s."), *Point.Id); return false; }
      }
      const FVector BasePosition = Position + FVector(0, 0, Offset*100.0);
      Base.Transform = FTransform(Rotation, BasePosition, FVector(FootScale));
      Base.Seed = static_cast<int32>(Seed);
      Position = BasePosition + FVector(0, 0, SoilHeight*FootScale*100.0/Hit.ImpactNormal.Z);
      Footing = MoveTemp(Base);
    }
    Point.Transform = FTransform(FRotator(0, Yaw, 0), Position, FVector(Scale));
    Point.Seed = static_cast<int32>(Seed);
    Ids.Add(Point.Id);
    Candidate.Add(MoveTemp(Point));
    if (Footing.IsSet())
    {
      if (Ids.Contains(Footing->Id)) { Status = TEXT("Duplicate footing ID."); return false; }
      Ids.Add(Footing->Id);
      Candidate.Add(MoveTemp(Footing.GetValue()));
    }
  }
  Modify();
  Points = MoveTemp(Candidate);
  FBox Bounds(ForceInit);
  for (const auto& Point : Points) Bounds += Point.Transform.GetLocation();
  if (!Bounds.IsValid) Bounds = FBox(FVector(-100), FVector(100));
  Bounds = Bounds.ExpandBy(1000.0);
  GenerationBounds->SetWorldLocation(Bounds.GetCenter());
  GenerationBounds->SetBoxExtent(Bounds.GetExtent());
  bPreviewReady = false;
  Status = FString::Printf(TEXT("Imported %d validated points. Preview, then Bake."), Points.Num());
  return true;
}

bool AProceduralVegetationTool::IsGenerating() const { return PCG && PCG->IsGenerating(); }

void AProceduralVegetationTool::TickCommandletGeneration()
{
  if (!IsRunningCommandlet()) return;
  FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
  if (UPCGSubsystem* Subsystem = GetWorld()->GetSubsystem<UPCGSubsystem>()) Subsystem->Tick(0.016f);
}

void AProceduralVegetationTool::Preview()
{
  if (IsGenerating()) { Status = TEXT("Generation already running."); return; }
  UPCGGraphInterface* LoadedGraph = Graph.LoadSynchronous();
  if (!LoadedGraph) { Status = TEXT("Missing PCG graph; run setup_vegetation_tool.py."); return; }
  Modify();
  bPreviewReady = false;
  PCG->CleanupLocalImmediate(true);
  for (UInstancedStaticMeshComponent* C : BakedComponents) if (C) C->SetVisibility(false);
  PCG->SetGraph(LoadedGraph);
#if WITH_EDITOR
  // DirtyGenerated updates editor caches and is absent from packaged PCG.
  // GenerateLocal(true) below already forces runtime regeneration.
  PCG->DirtyGenerated(EPCGComponentDirtyFlag::All);
#endif
  Status = TEXT("Generating PCG vegetation preview...");
  PCG->GenerateLocal(true);
}

int32 AProceduralVegetationTool::PreviewInstanceCount() const
{
  int32 Count = 0;
  TArray<UInstancedStaticMeshComponent*> Components;
  GetComponents(Components);
  for (auto* C : Components) if (!BakedComponents.Contains(C)) Count += C->GetInstanceCount();
  return Count;
}

void AProceduralVegetationTool::OnGenerated(UPCGComponent* Component)
{
  const int32 Count = PreviewInstanceCount();
  bPreviewReady = Count == Points.Num();
  ATagger::TagActor(*this, true);
  Status = FString::Printf(TEXT("PCG preview: %d/%d instances. %s"), Count, Points.Num(),
      bPreviewReady ? TEXT("Ready to bake.") : TEXT("Incomplete; inspect PCG errors."));
  if (!bPreviewReady) for (UInstancedStaticMeshComponent* C : BakedComponents) if (C) C->SetVisibility(true);
}

bool AProceduralVegetationTool::Bake()
{
  if (IsGenerating() || !bPreviewReady || PreviewInstanceCount() != Points.Num())
  { Status = TEXT("Generate a complete preview before baking."); return false; }
  Modify();
  // Upgrade existing tool actors too: static instances cannot attach to a
  // movable root, and must follow the region actor's transform after baking.
  RootComponent->Modify();
  RootComponent->SetMobility(EComponentMobility::Static);
  TArray<UInstancedStaticMeshComponent*> PreviewComponents;
  GetComponents(PreviewComponents);
  TArray<TObjectPtr<UInstancedStaticMeshComponent>> NewBaked;
  int32 Count = 0;
  for (auto* Source : PreviewComponents)
  {
    if (BakedComponents.Contains(Source) || Source->GetInstanceCount() == 0) continue;
    auto* Dest = NewObject<UInstancedStaticMeshComponent>(this, NAME_None, RF_Transactional);
    Dest->SetStaticMesh(Source->GetStaticMesh());
    Dest->SetMobility(EComponentMobility::Static);
    Dest->SetupAttachment(RootComponent);
    Dest->SetRelativeTransform(FTransform::Identity);
    Dest->SetCollisionProfileName(TEXT("BlockAll"));
    Dest->SetGenerateOverlapEvents(false);
    Dest->SetCanEverAffectNavigation(true);
    for (int32 M=0; M<Source->GetNumMaterials(); ++M) Dest->SetMaterial(M, Source->GetMaterial(M));
    AddInstanceComponent(Dest);
    Dest->RegisterComponent();
    for (int32 I=0; I<Source->GetInstanceCount(); ++I)
    {
      FTransform Transform;
      Source->GetInstanceTransform(I, Transform, true);
      Dest->AddInstance(Transform, true);
      ++Count;
    }
    NewBaked.Add(Dest);
  }
  for (UInstancedStaticMeshComponent* Old : BakedComponents) if (Old) { RemoveInstanceComponent(Old); Old->DestroyComponent(); }
  BakedComponents = MoveTemp(NewBaked);
  BakedInstanceCount = Count;
  PCG->CleanupLocalImmediate(true);
  bPreviewReady = false;
  ATagger::TagActor(*this, true);
  MarkPackageDirty();
  Status = FString::Printf(TEXT("Baked %d plant and surround instances. Save the level."), Count);
  return Count == Points.Num();
}

void AProceduralVegetationTool::ClearGenerated()
{
  if (IsGenerating()) { Status = TEXT("Wait for generation to finish before clearing."); return; }
  Modify();
  PCG->CleanupLocalImmediate(true);
  for (UInstancedStaticMeshComponent* C : BakedComponents) if (C) { RemoveInstanceComponent(C); C->DestroyComponent(); }
  BakedComponents.Empty();
  BakedInstanceCount = 0;
  bPreviewReady = false;
  Status = TEXT("Generated vegetation cleared; imported plan retained.");
  MarkPackageDirty();
}

class FPCGVegetationPlanElement : public IPCGElement
{
  virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override { return true; }
  virtual bool IsCacheable(const UPCGSettings* Settings) const override { return false; }
  virtual bool ExecuteInternal(FPCGContext* Context) const override
  {
    const auto* Tool = Cast<AProceduralVegetationTool>(Context->GetTargetActor(nullptr));
    if (!Tool) return true;
    UPCGPointData* Data = FPCGContext::NewObject_AnyThread<UPCGPointData>(Context);
    auto* Mesh = Data->Metadata->CreateAttribute<FString>(TEXT("Mesh"), FString(), false, false);
    auto* Id = Data->Metadata->CreateAttribute<FString>(TEXT("SourceId"), FString(), false, false);
    auto* Provenance = Data->Metadata->CreateAttribute<FString>(TEXT("Provenance"), FString(), false, false);
    auto& Output = Data->GetMutablePoints();
    for (const auto& Record : Tool->Points)
    {
      FPCGPoint& Point = Output.Emplace_GetRef();
      Point.Transform = Record.Transform;
      Point.Seed = Record.Seed;
      Point.Density = 1.0f;
      Point.MetadataEntry = Data->Metadata->AddEntry();
      Mesh->SetValue(Point.MetadataEntry, Record.Mesh.ToSoftObjectPath().ToString());
      Id->SetValue(Point.MetadataEntry, Record.Id);
      Provenance->SetValue(Point.MetadataEntry, Record.Provenance);
    }
    FPCGTaggedData& Tagged = Context->OutputData.TaggedData.Emplace_GetRef();
    Tagged.Pin = PCGPinConstants::DefaultOutputLabel;
    Tagged.Tags.Add(TEXT("Vegetation"));
    Tagged.Data = Data;
    return true;
  }
};

FPCGElementPtr UPCGVegetationPlanSettings::CreateElement() const { return MakeShared<FPCGVegetationPlanElement>(); }
