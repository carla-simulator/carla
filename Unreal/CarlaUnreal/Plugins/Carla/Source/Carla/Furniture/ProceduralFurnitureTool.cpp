#include "ProceduralFurnitureTool.h"

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

AProceduralFurnitureTool::AProceduralFurnitureTool()
{
  RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
  RootComponent->SetMobility(EComponentMobility::Static);
  GenerationBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("GenerationBounds"));
  GenerationBounds->SetupAttachment(RootComponent);
  GenerationBounds->SetBoxExtent(FVector(100));
  GenerationBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  GenerationBounds->SetHiddenInGame(true);
  GenerationBounds->SetCanEverAffectNavigation(false);
  PCG = CreateDefaultSubobject<UPCGComponent>(TEXT("FurniturePCG"));
  PCG->GenerationTrigger = EPCGComponentGenerationTrigger::GenerateOnDemand;
  PCG->OnPCGGraphGeneratedExternal.AddDynamic(this, &AProceduralFurnitureTool::OnGenerated);
  Graph = TSoftObjectPtr<UPCGGraphInterface>(FSoftObjectPath(TEXT("/Game/Carla/Blueprints/PCG/FurnitureTool/PCG_TwinFurniture.PCG_TwinFurniture")));
  PrimaryActorTick.bCanEverTick = false;
#if WITH_EDITORONLY_DATA
  bIsSpatiallyLoaded = false;
#endif
}

bool AProceduralFurnitureTool::ImportPlan()
{
  if (IsGenerating()) { Status = TEXT("Wait for PCG generation to finish."); return false; }
  FString Text;
  TSharedPtr<FJsonObject> Root;
  if (!FFileHelper::LoadFileToString(Text, *PlanFile.FilePath) ||
      !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root) || !Root.IsValid())
  { Status = TEXT("Cannot read furniture plan JSON."); return false; }
  FString Schema, Coordinates;
  const TArray<TSharedPtr<FJsonValue>>* Entries = nullptr;
  if (!Root->TryGetStringField(TEXT("schema"), Schema) || Schema != TEXT("twin-furniture-plan/1") ||
      !Root->TryGetStringField(TEXT("coordinate_system"), Coordinates) || Coordinates != TEXT("carla-metres") ||
      !Root->TryGetArrayField(TEXT("points"), Entries) || Entries->Num() > 200000)
  { Status = TEXT("Unsupported plan schema, coordinates or point count."); return false; }
  TArray<FCarlaFurniturePoint> Candidate;
  TSet<FString> Ids;
#if WITH_EDITOR
  // Spatial ground actors can have just been loaded by the commandlet. Their
  // asynchronous mesh compilation must finish before collision bodies exist.
  FStaticMeshCompilingManager::Get().FinishAllCompilation();
#endif
  FCollisionQueryParams Query(SCENE_QUERY_STAT(FurnitureGround), true, this);
  for (TActorIterator<AProceduralFurnitureTool> It(GetWorld()); It; ++It) Query.AddIgnoredActor(*It);
  for (const auto& Entry : *Entries)
  {
    const TSharedPtr<FJsonObject>* ObjPtr = nullptr;
    if (!Entry->TryGetObject(ObjPtr)) { Status = TEXT("Invalid point object."); return false; }
    const auto& Obj = *ObjPtr;
    FCarlaFurniturePoint Point;
    FString MeshPath, State, Kind;
    double X, Y, Z, Scale, Yaw, Seed, SupportDelta;
    if (!Obj->TryGetStringField(TEXT("id"), Point.Id) || Point.Id.IsEmpty() || Ids.Contains(Point.Id) ||
        !Obj->TryGetStringField(TEXT("status"), State) || State != TEXT("accepted") ||
        !Obj->TryGetStringField(TEXT("mesh"), MeshPath) ||
        !Obj->TryGetStringField(TEXT("source"), Point.Provenance) ||
        !Obj->TryGetStringField(TEXT("kind"), Kind) ||
        !((Kind == TEXT("bench") && MeshPath == TEXT("/Game/Carla/Static/Static/SM_Bench01")) ||
          (Kind == TEXT("bin") && MeshPath == TEXT("/Game/Carla/Static/Static/SM_TrashCan03")) ||
          (Kind == TEXT("bus_shelter") && MeshPath == TEXT("/Game/Carla/Static/Static/Materials/BusStop02/SM_BusStop02")) ||
          (Kind == TEXT("bus_glass") && MeshPath == TEXT("/Game/Carla/Static/Static/Materials/BusStop02/SM_BusStop02_Glass")) ||
          (Kind == TEXT("bus_pole") && MeshPath == TEXT("/Game/Carla/Static/Pole/SM_Pole08")) ||
          (Kind == TEXT("bus_panel") && MeshPath == TEXT("/Game/Carla/Static/TrafficSign/TwinFurniture/SM_BusStopMarker")) ||
          (Kind == TEXT("banner_pole") && MeshPath == TEXT("/Game/Carla/Static/Pole/SM_Pole05")) ||
          (Kind == TEXT("banner") && MeshPath == TEXT("/Game/Carla/Static/Other/Banner/SM_Banner01"))) ||
        !Obj->TryGetNumberField(TEXT("max_support_delta_m"), SupportDelta) ||
        !FMath::IsFinite(SupportDelta) || SupportDelta <= 0 || SupportDelta > 0.05 ||
        !Obj->TryGetNumberField(TEXT("x"), X) || !Obj->TryGetNumberField(TEXT("y"), Y) ||
        !Obj->TryGetNumberField(TEXT("z"), Z) || !Obj->TryGetNumberField(TEXT("scale"), Scale) ||
        !Obj->TryGetNumberField(TEXT("yaw"), Yaw) || !Obj->TryGetNumberField(TEXT("seed"), Seed) ||
        !FMath::IsFinite(X) || !FMath::IsFinite(Y) || !FMath::IsFinite(Z) ||
        !FMath::IsFinite(Scale) || Scale != 1.0 || !FMath::IsFinite(Yaw) ||
        !FMath::IsFinite(Seed) || Seed < 0 || Seed > MAX_int32)
    { Status = TEXT("Invalid or duplicate furniture point; previous plan retained."); return false; }
    if (!MeshPath.Contains(TEXT("."))) MeshPath += TEXT(".") + FPackageName::GetShortName(MeshPath);
    Point.Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(MeshPath));
    if (!Point.Mesh.LoadSynchronous())
    { Status = FString::Printf(TEXT("Missing mesh: %s; previous plan retained."), *MeshPath); return false; }
    const FBoxSphereBounds MeshBounds = Point.Mesh.Get()->GetBounds();
    // Check catalog bounds against loaded assets, including off-center pivots.
    for (const FString& Field : {FString(TEXT("min_m")), FString(TEXT("max_m"))})
    {
      const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
      if (!Obj->TryGetArrayField(Field, Values) || Values->Num() != 3)
      { Status = TEXT("Missing measured mesh bounds."); return false; }
      const FVector Expected = Field == TEXT("min_m") ? MeshBounds.Origin-MeshBounds.BoxExtent : MeshBounds.Origin+MeshBounds.BoxExtent;
      for (int Axis=0; Axis<3; ++Axis)
      {
        double Value;
        if (!(*Values)[Axis]->TryGetNumber(Value) || !FMath::IsFinite(Value) || FMath::Abs(Value*100-Expected[Axis]) > 0.1)
        { Status = TEXT("Catalog bounds changed; regenerate the furniture plan."); return false; }
      }
    }
    FString ParentId;
    Obj->TryGetStringField(TEXT("parent_id"), ParentId);
    const bool bAttached = Kind == TEXT("bus_glass") || Kind == TEXT("bus_panel") || Kind == TEXT("banner");
    if (bAttached != !ParentId.IsEmpty())
    { Status = TEXT("Furniture attachment is missing or unexpected."); return false; }
    if (bAttached)
    {
      FString GroupId;
      const FString ParentKind = Kind == TEXT("bus_glass") ? TEXT("bus_shelter") :
          Kind == TEXT("bus_panel") ? TEXT("bus_pole") : TEXT("banner_pole");
      const double Height = Kind == TEXT("bus_glass") ? 0.0 : Kind == TEXT("bus_panel") ? 2.05 : 4.47;
      const FVector MountOffset(0,Kind == TEXT("bus_panel") ? 0.025 : 0.0,Height);
      const double MountYaw = 0.0;
      const TArray<TSharedPtr<FJsonValue>>* Offset = nullptr;
      if (!Obj->TryGetStringField(TEXT("group_id"), GroupId) || ParentId != GroupId+TEXT("/")+ParentKind ||
          Point.Id != GroupId+TEXT("/")+Kind || !Obj->TryGetArrayField(TEXT("attachment_m"), Offset) || Offset->Num()!=3)
      { Status = TEXT("Invalid furniture assembly parent."); return false; }
      for (int Axis=0; Axis<3; ++Axis)
      {
        double Value;
        if (!(*Offset)[Axis]->TryGetNumber(Value) || !FMath::IsFinite(Value) || FMath::Abs(Value-MountOffset[Axis])>0.0001)
        { Status = TEXT("Unreviewed furniture attachment offset."); return false; }
      }
      const FCarlaFurniturePoint* Parent = Candidate.FindByPredicate([&](const FCarlaFurniturePoint& P){ return P.Id==ParentId; });
      if (!Parent || FVector2D(Parent->Transform.TransformPosition(MountOffset*100)-FVector(X,Y,Z)*100).Size()>0.1 ||
          FMath::Abs(FMath::FindDeltaAngleDegrees(Parent->Transform.Rotator().Yaw+MountYaw,Yaw))>0.1)
      { Status = FString::Printf(TEXT("Assembly parent pose mismatch for %s: parent=%s yaw=%.4f expected=%.4f"), *Point.Id, *ParentId, Parent ? Parent->Transform.Rotator().Yaw : 0.0, Yaw); return false; }
      // Glass retains its authored elevated bounds. Banners and panels use the
      // reviewed support-relative mount; none of these is independently grounded.
      Point.Transform = FTransform(Parent->Transform.GetRotation()*FRotator(0,MountYaw,0).Quaternion(),
                                   Parent->Transform.TransformPosition(MountOffset*100),FVector(1));
      Point.Seed = static_cast<int32>(Seed);
      Ids.Add(Point.Id);
      Candidate.Add(MoveTemp(Point));
      continue;
    }
    FVector Position = FVector(X,Y,Z)*100.0;
    const FRotator Rotation(0,Yaw,0);
    FQuat GroundRotation = Rotation.Quaternion();
    double Heights[3][3];
    double MinZ = TNumericLimits<double>::Max(), MaxZ = -TNumericLimits<double>::Max();
    // Trace the footprint corners, edge midpoints and center. Never snap onto
    // a road, another prop, a roof or a different elevation layer.
    for (int I=-1; I<=1; ++I) for (int J=-1; J<=1; ++J)
    {
      const FVector Local(MeshBounds.Origin.X+I*MeshBounds.BoxExtent.X,
                          MeshBounds.Origin.Y+J*MeshBounds.BoxExtent.Y,0);
      const FVector Probe = Position+Rotation.RotateVector(Local);
      FHitResult Hit;
      const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Probe+FVector(0,0,30), Probe-FVector(0,0,30), ECC_Visibility, Query);
      const auto* Ground = bHit ? Cast<UStaticMeshComponent>(Hit.GetComponent()) : nullptr;
      if (!Ground || !Ground->GetStaticMesh() ||
          !Ground->GetStaticMesh()->GetPathName().StartsWith(TEXT("/Game/Carla/Static/SideWalk/")) ||
          FMath::Abs(Hit.ImpactPoint.Z-Position.Z)>15.0)
      { Status = FString::Printf(TEXT("Unsupported furniture footprint: %s"), *Point.Id); return false; }
      Heights[I+1][J+1] = Hit.ImpactPoint.Z;
      MinZ = FMath::Min(MinZ, Hit.ImpactPoint.Z);
      MaxZ = FMath::Max(MaxZ, Hit.ImpactPoint.Z);
    }
    double GroundZ = (MinZ+MaxZ)/2;
    if (Kind == TEXT("bus_shelter"))
    {
      // Fit a mild pavement plane across the shelter's full support footprint.
      // This preserves glass alignment while avoiding floating end legs.
      const double SX = (Heights[2][1]-Heights[0][1])/(2*MeshBounds.BoxExtent.X);
      const double SY = (Heights[1][2]-Heights[1][0])/(2*MeshBounds.BoxExtent.Y);
      double Residual = 0;
      for (int I=-1; I<=1; ++I) for (int J=-1; J<=1; ++J)
        Residual = FMath::Max(Residual,FMath::Abs(Heights[I+1][J+1]-Heights[1][1]-I*MeshBounds.BoxExtent.X*SX-J*MeshBounds.BoxExtent.Y*SY));
      if (FMath::RadiansToDegrees(FMath::Atan(FMath::Sqrt(SX*SX+SY*SY)))>3 || Residual>SupportDelta*100)
      { Status = FString::Printf(TEXT("Uneven furniture support: %s (shelter plane)"), *Point.Id); return false; }
      const FVector Normal = Rotation.RotateVector(FVector(-SX,-SY,1).GetSafeNormal());
      GroundRotation = FRotationMatrix::MakeFromZX(Normal,Rotation.Vector()).ToQuat();
      GroundZ = Heights[1][1]-MeshBounds.Origin.X*SX-MeshBounds.Origin.Y*SY;
    }
    else if (MaxZ-MinZ > SupportDelta*100)
    { Status = FString::Printf(TEXT("Uneven furniture support: %s (%.3f m)"), *Point.Id, (MaxZ-MinZ)/100); return false; }
    Position.Z = GroundZ - (MeshBounds.Origin.Z-MeshBounds.BoxExtent.Z);
    Point.Transform = FTransform(GroundRotation, Position, FVector(Scale));
    Point.Seed = static_cast<int32>(Seed);
    Ids.Add(Point.Id);
    Candidate.Add(MoveTemp(Point));
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

bool AProceduralFurnitureTool::IsGenerating() const { return PCG && PCG->IsGenerating(); }

void AProceduralFurnitureTool::TickCommandletGeneration()
{
  if (!IsRunningCommandlet()) return;
  FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
  if (UPCGSubsystem* Subsystem = GetWorld()->GetSubsystem<UPCGSubsystem>()) Subsystem->Tick(0.016f);
}

void AProceduralFurnitureTool::Preview()
{
  if (IsGenerating()) { Status = TEXT("Generation already running."); return; }
  UPCGGraphInterface* LoadedGraph = Graph.LoadSynchronous();
  if (!LoadedGraph) { Status = TEXT("Missing PCG graph; run setup_furniture_tool.py."); return; }
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
  Status = TEXT("Generating PCG furniture preview...");
  PCG->GenerateLocal(true);
}

int32 AProceduralFurnitureTool::PreviewInstanceCount() const
{
  int32 Count = 0;
  TArray<UInstancedStaticMeshComponent*> Components;
  GetComponents(Components);
  for (auto* C : Components) if (!BakedComponents.Contains(C)) Count += C->GetInstanceCount();
  return Count;
}

void AProceduralFurnitureTool::OnGenerated(UPCGComponent* Component)
{
  const int32 Count = PreviewInstanceCount();
  bPreviewReady = Count == Points.Num();
  ATagger::TagActor(*this, true);
  Status = FString::Printf(TEXT("PCG preview: %d/%d instances. %s"), Count, Points.Num(),
      bPreviewReady ? TEXT("Ready to bake.") : TEXT("Incomplete; inspect PCG errors."));
  if (!bPreviewReady) for (UInstancedStaticMeshComponent* C : BakedComponents) if (C) C->SetVisibility(true);
}

bool AProceduralFurnitureTool::Bake()
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
  Status = FString::Printf(TEXT("Baked %d furniture instances. Save the level."), Count);
  return Count == Points.Num();
}

void AProceduralFurnitureTool::ClearGenerated()
{
  if (IsGenerating()) { Status = TEXT("Wait for generation to finish before clearing."); return; }
  Modify();
  PCG->CleanupLocalImmediate(true);
  for (UInstancedStaticMeshComponent* C : BakedComponents) if (C) { RemoveInstanceComponent(C); C->DestroyComponent(); }
  BakedComponents.Empty();
  BakedInstanceCount = 0;
  bPreviewReady = false;
  Status = TEXT("Generated furniture cleared; imported plan retained.");
  MarkPackageDirty();
}

class FPCGFurniturePlanElement : public IPCGElement
{
  virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override { return true; }
  virtual bool IsCacheable(const UPCGSettings* Settings) const override { return false; }
  virtual bool ExecuteInternal(FPCGContext* Context) const override
  {
    const auto* Tool = Cast<AProceduralFurnitureTool>(Context->GetTargetActor(nullptr));
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
    Tagged.Tags.Add(TEXT("Furniture"));
    Tagged.Data = Data;
    return true;
  }
};

FPCGElementPtr UPCGFurniturePlanSettings::CreateElement() const { return MakeShared<FPCGFurniturePlanElement>(); }
