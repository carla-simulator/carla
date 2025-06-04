// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MapGenFunctionLibrary.h"

#include "Carla/Actor/LevelActor/InstancedStaticMeshActor.h"

#include <util/ue-header-guard-begin.h>
#include "AssetRegistry/AssetRegistryModule.h"
#include "Materials/MaterialInstance.h"
#include "StaticMeshAttributes.h"
#include "RenderingThread.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/SavePackage.h"
#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#include "Editor/Transactor.h"
#endif
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include <util/ue-header-guard-end.h>

DEFINE_LOG_CATEGORY(LogCarlaMapGenFunctionLibrary);
static const float OSMToCentimetersScaleFactor = 100.0f;

FMeshDescription UMapGenFunctionLibrary::BuildMeshDescriptionFromData(
  const FProceduralCustomMesh& Data,
  const TArray<FProcMeshTangent>& ParamTangents,
  UMaterialInstance* MaterialInstance  )
{

  int32 VertexCount = Data.Vertices.Num();
  int32 VertexInstanceCount = Data.Triangles.Num();
  int32 PolygonCount = Data.Vertices.Num()/3;

  FMeshDescription MeshDescription;
  FStaticMeshAttributes AttributeGetter(MeshDescription);
  AttributeGetter.Register();

  auto PolygonGroupNames = AttributeGetter.GetPolygonGroupMaterialSlotNames();
  auto VertexPositions = AttributeGetter.GetVertexPositions();
  auto Tangents = AttributeGetter.GetVertexInstanceTangents();
  auto BinormalSigns = AttributeGetter.GetVertexInstanceBinormalSigns();
  auto Normals = AttributeGetter.GetVertexInstanceNormals();
  auto Colors = AttributeGetter.GetVertexInstanceColors();
  auto UVs = AttributeGetter.GetVertexInstanceUVs();

  // Calculate the totals for each ProcMesh element type
  FPolygonGroupID PolygonGroupForSection;
  MeshDescription.ReserveNewVertices(VertexCount);
  MeshDescription.ReserveNewVertexInstances(VertexInstanceCount);
  MeshDescription.ReserveNewPolygons(PolygonCount);
  MeshDescription.ReserveNewEdges(PolygonCount * 2);
  UVs.SetNumChannels(4);

  // Create Materials
  TMap<UMaterialInterface*, FPolygonGroupID> UniqueMaterials;
	const int32 NumSections = 1;
	UniqueMaterials.Reserve(1);
  FPolygonGroupID NewPolygonGroup = MeshDescription.CreatePolygonGroup();

  if( MaterialInstance != nullptr ){
    UMaterialInterface *Material = MaterialInstance;
    UniqueMaterials.Add(Material, NewPolygonGroup);
    PolygonGroupNames[NewPolygonGroup] = Material->GetFName();
  }else{
    UE_LOG(LogCarla, Error, TEXT("MaterialInstance is nullptr"));
  }
  PolygonGroupForSection = NewPolygonGroup;



  // Create the vertex
  int32 NumVertex = Data.Vertices.Num();
  TMap<int32, FVertexID> VertexIndexToVertexID;
  VertexIndexToVertexID.Reserve(NumVertex);
  for (int32 VertexIndex = 0; VertexIndex < NumVertex; ++VertexIndex)
  {
    auto &Vert = Data.Vertices[VertexIndex];
    auto VertexID = MeshDescription.CreateVertex();
    VertexPositions[VertexID] = FVector3f(Vert);
    VertexIndexToVertexID.Add(VertexIndex, VertexID);
  }

  // Create the VertexInstance
  int32 NumIndices = Data.Triangles.Num();
  int32 NumTri = NumIndices / 3;
  TMap<int32, FVertexInstanceID> IndiceIndexToVertexInstanceID;
  IndiceIndexToVertexInstanceID.Reserve(NumVertex);
  for (int32 IndiceIndex = 0; IndiceIndex < NumIndices; IndiceIndex++)
  {
    const int32 VertexIndex = Data.Triangles[IndiceIndex];
    const FVertexID VertexID = VertexIndexToVertexID[VertexIndex];
    const FVertexInstanceID VertexInstanceID =
    MeshDescription.CreateVertexInstance(VertexID);
    IndiceIndexToVertexInstanceID.Add(IndiceIndex, VertexInstanceID);
    Normals[VertexInstanceID] = FVector3f(Data.Normals[VertexIndex]);

    if(ParamTangents.Num() == Data.Vertices.Num())
    {
        Tangents[VertexInstanceID] = FVector3f(ParamTangents[VertexIndex].TangentX);
      BinormalSigns[VertexInstanceID] =
        ParamTangents[VertexIndex].bFlipTangentY ? -1.f : 1.f;
    }else{

    }
    Colors[VertexInstanceID] = FLinearColor(0,0,0);
    if(Data.UV0.Num() == Data.Vertices.Num())
    {
      UVs.Set(VertexInstanceID, 0, FVector2f(Data.UV0[VertexIndex]));
    }else{
      UVs.Set(VertexInstanceID, 0, FVector2f(0,0));
    }
    UVs.Set(VertexInstanceID, 1, FVector2f(0,0));
    UVs.Set(VertexInstanceID, 2, FVector2f(0,0));
    UVs.Set(VertexInstanceID, 3, FVector2f(0,0));
  }

  for (int32 TriIdx = 0; TriIdx < NumTri; TriIdx++)
  {
    FVertexID VertexIndexes[3];
    TArray<FVertexInstanceID> VertexInstanceIDs;
    VertexInstanceIDs.SetNum(3);

    for (int32 CornerIndex = 0; CornerIndex < 3; ++CornerIndex)
    {
      const int32 IndiceIndex = (TriIdx * 3) + CornerIndex;
      const int32 VertexIndex = Data.Triangles[IndiceIndex];
      VertexIndexes[CornerIndex] = VertexIndexToVertexID[VertexIndex];
      VertexInstanceIDs[CornerIndex] =
        IndiceIndexToVertexInstanceID[IndiceIndex];
    }

    // Insert a polygon into the mesh
    MeshDescription.CreatePolygon(NewPolygonGroup, VertexInstanceIDs);

  }

  return MeshDescription;
}

UStaticMesh* UMapGenFunctionLibrary::CreateMesh(
    const FProceduralCustomMesh& Data,
    const TArray<FProcMeshTangent>& ParamTangents,
    UMaterialInstance* MaterialInstance,
    FString MapName,
    FString FolderName,
    FName MeshName)
{
  IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

  UStaticMesh::FBuildMeshDescriptionsParams Params;
  Params.bBuildSimpleCollision = true;

  FString PackageName = "/Game/CustomMaps/" + MapName + "/Static/" + FolderName + "/" + MeshName.ToString();

  if (!PlatformFile.DirectoryExists(*PackageName))
  {
    //PlatformFile.CreateDirectory(*PackageName);
  }


  FMeshDescription Description = BuildMeshDescriptionFromData(Data,ParamTangents, MaterialInstance);

  if (Description.Polygons().Num() > 0)
  {
    UPackage* Package = CreatePackage(*PackageName);
    check(Package);
    UStaticMesh* Mesh = NewObject<UStaticMesh>( Package, MeshName, RF_Public | RF_Standalone);

    Mesh->InitResources();
    Mesh->SetLightingGuid(FGuid::NewGuid());
    Mesh->GetStaticMaterials().Add(FStaticMaterial(MaterialInstance));
    Mesh->BuildFromMeshDescriptions({ &Description }, Params);
    Mesh->CreateBodySetup();
    Mesh->GetBodySetup()->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;
    Mesh->GetBodySetup()->CreatePhysicsMeshes();
    // Build mesh from source
    Mesh->NeverStream = false;
    TArray<UObject*> CreatedAssets;
    CreatedAssets.Add(Mesh);

    // Notify asset registry of new asset
    FAssetRegistryModule::AssetCreated(Mesh);

    /*

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags =
      EObjectFlags::RF_Public |
      EObjectFlags::RF_Standalone;
    SaveArgs.Error = GError;
    SaveArgs.bForceByteSwapping = true;
    SaveArgs.bWarnOfLongFilename = true;
    SaveArgs.SaveFlags = SAVE_NoError;

    UPackage::SavePackage(
      Package, 
      Mesh,
      *(MeshName.ToString()),
      SaveArgs);

    */

    Package->MarkPackageDirty();
    return Mesh;
  }
  return nullptr;
}

FVector2D UMapGenFunctionLibrary::GetTransversemercProjection(float lat, float lon, float lat0, float lon0)
{
  // earth radius in m
  const float R = 6373000.0f;
  float latt = FMath::DegreesToRadians(lat);
  float lonn  = FMath::DegreesToRadians(lon - lon0);
  float latt0 = FMath::DegreesToRadians(lat0);
  float eps = atan(tan(latt)/cos(lonn));
  float nab = asinh(sin(lonn)/sqrt(tan(latt)*tan(latt)+cos(lonn)*cos(lonn)));
  float x = R*nab;
  float y = R*eps;
  float eps0 = atan(tan(latt0)/cos(0));
  float nab0 = asinh(sin(0)/sqrt(tan(latt0)*tan(latt0)+cos(0)*cos(0)));
  float x0 = R*nab0;
  float y0 = R*eps0;
  FVector2D Result = FVector2D(x, -(y - y0)) * OSMToCentimetersScaleFactor;

  return Result;
}

void UMapGenFunctionLibrary::SetThreadToSleep(float seconds)
{
  //FGenericPlatformProcess::Sleep(seconds);
}

void UMapGenFunctionLibrary::FlushRenderingCommandsInBlueprint()
{
    FlushRenderingCommands();
    FlushPendingDeleteRHIResources_GameThread();
}

#if WITH_EDITOR
extern UNREALED_API UEditorEngine* GEditor;
#endif

void UMapGenFunctionLibrary::CleanupGEngine(){
#if WITH_EDITOR
  GEngine->PerformGarbageCollectionAndCleanupActors();
  FText TransResetText(FText::FromString("Clean up after Move actors to sublevels"));
  if ( GEditor->Trans )
  {
    GEditor->Trans->Reset(TransResetText);
    GEditor->Cleanse(true, true, TransResetText);
  }
#endif
}

TArray<AInstancedStaticMeshActor*> UMapGenFunctionLibrary::ChangeStaticMeshesInTheLevelForInstancedStaticMeshes(UWorld* World, TArray<UStaticMesh*> Filter, int MinNumOfInstancesToBeChanged)
{
  TArray<AInstancedStaticMeshActor*> Result;
  TArray<AActor*> FoundActors;
  UGameplayStatics::GetAllActorsOfClass(World,  AStaticMeshActor::StaticClass(), FoundActors);
  TMap<UStaticMesh*, TArray<AStaticMeshActor*>> ActorsToCheckIfReplacedMap;
  for(AActor* CurrentActor : FoundActors )
  {
    AStaticMeshActor* CurrentStaticMeshActor = Cast<AStaticMeshActor>(CurrentActor);
    UStaticMeshComponent* CurrentStaticMeshComponent = CurrentStaticMeshActor->GetStaticMeshComponent();
    UStaticMesh* CurrentStaticMesh = CurrentStaticMeshComponent->GetStaticMesh();
    if( Filter.IsEmpty() || Filter.Contains(CurrentStaticMesh) )
    {
      TArray<AStaticMeshActor*>& Instances = ActorsToCheckIfReplacedMap.FindOrAdd(CurrentStaticMesh);
      Instances.Add(CurrentStaticMeshActor);
    }
  }

  for(auto CurrentPair : ActorsToCheckIfReplacedMap)
  {

    if(CurrentPair.Value.Num() > MinNumOfInstancesToBeChanged)
    {
      TArray<FTransform> TransformsToBeInstanced;
      for( AStaticMeshActor* SMActor : CurrentPair.Value )
      {
        TransformsToBeInstanced.Add(SMActor->GetActorTransform());
        SMActor->Destroy();
      }

      AInstancedStaticMeshActor* InstancedStaticMeshActor = World->SpawnActor<AInstancedStaticMeshActor>();
      if(InstancedStaticMeshActor)
      {
        InstancedStaticMeshActor->GetInstancedStaticMeshComponent()->SetStaticMesh(CurrentPair.Key);
        InstancedStaticMeshActor->GetInstancedStaticMeshComponent()->AddInstances(TransformsToBeInstanced, false, true);
        Result.Add(InstancedStaticMeshActor);
      }
    }
  }

  return Result;
}

TArray<AStaticMeshActor*> UMapGenFunctionLibrary::RevertStaticMeshesInTheLevelForInstancedStaticMeshes(UWorld* World, TArray<UStaticMesh*> Filter)
{
  TArray<AActor*> FoundActors;
  TArray<AStaticMeshActor*> Result;
  UGameplayStatics::GetAllActorsOfClass(World,  AInstancedStaticMeshActor::StaticClass(), FoundActors);
  for(AActor* CurrentActor : FoundActors )
  {
    AInstancedStaticMeshActor* InstancedStaticMeshActor = Cast<AInstancedStaticMeshActor>(CurrentActor);
    if(InstancedStaticMeshActor)
    {
      UStaticMesh* CurrentStaticMesh = InstancedStaticMeshActor->GetInstancedStaticMeshComponent()->GetStaticMesh();
      if( Filter.IsEmpty() || Filter.Contains(CurrentStaticMesh))
      {
        const TArray<FInstancedStaticMeshInstanceData>& Instances = InstancedStaticMeshActor->GetInstancedStaticMeshComponent()->PerInstanceSMData;
        for( FInstancedStaticMeshInstanceData CurrentInstanceData : Instances )
        {
          AStaticMeshActor* StaticMeshActor = World->SpawnActor<AStaticMeshActor>();
          StaticMeshActor->GetStaticMeshComponent()->SetStaticMesh(CurrentStaticMesh);
          StaticMeshActor->SetActorTransform(FTransform(CurrentInstanceData.Transform));
          Result.Add(StaticMeshActor);
        }
        InstancedStaticMeshActor->Destroy();
      }
    }
  }

  return Result;
}
