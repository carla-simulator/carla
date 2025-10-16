// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaExporter.h"
#include "CarlaExporterCommands.h"

#include "Misc/MessageDialog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "Engine/Selection.h"
#include "Engine/StaticMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConvexElem.h"
#include "LevelEditor.h"
#include "EngineUtils.h"
#include "Chaos/TriangleMeshImplicitObject.h"

#include <fstream>
#include <sstream>

static const FName CarlaExporterTabName("CarlaExporter");

#define LOCTEXT_NAMESPACE "FCarlaExporterModule"

// Helper function to check if a component is a building mesh
static bool IsBuilding(UInstancedStaticMeshComponent* comp)
{
  if (!comp || !comp->GetStaticMesh()) return false;
  FString MeshPath = comp->GetStaticMesh()->GetPathName();
  return MeshPath.Contains(TEXT("/Building/")) || MeshPath.Contains(TEXT("/Buildings/"));
}

void FCarlaExporterModule::StartupModule()
{
  // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

  FCarlaExporterCommands::Register();

  PluginCommands = MakeShareable(new FUICommandList);

  PluginCommands->MapAction(
    FCarlaExporterCommands::Get().PluginActionExportAll,
    FExecuteAction::CreateRaw(this, &FCarlaExporterModule::PluginButtonClicked),
    FCanExecuteAction());

  FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

  {
    TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
    MenuExtender->AddMenuExtension("FileActors",
      EExtensionHook::After,
      PluginCommands,
      FMenuExtensionDelegate::CreateRaw(this, &FCarlaExporterModule::AddMenuExtension));

    LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
  }
}

void FCarlaExporterModule::ShutdownModule()
{
  // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
  // we call this function before unloading the module.
  FCarlaExporterCommands::Unregister();
}

void FCarlaExporterModule::PluginButtonClicked()
{
  UWorld* World = GEditor->GetEditorWorldContext().World();
  if (!World) return;

  // get all selected objects (if any)
  TArray<UObject*> BP_Actors;
  USelection* CurrentSelection = GEditor->GetSelectedActors();
  int32 SelectionNum = CurrentSelection->GetSelectedObjects(AActor::StaticClass(), BP_Actors);

  // if no selection, then get all objects
  if (SelectionNum == 0)
  {
    for (TActorIterator<AActor> it(World); it; ++it)
      BP_Actors.Add(Cast<UObject>(*it));
  }

  // get target path
  FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());
  // build final name
  std::ostringstream name;
  name << TCHAR_TO_UTF8(*Path) << "/" << TCHAR_TO_UTF8(*World->GetMapName()) << ".obj";
  // create the file
  std::ofstream f(name.str());

  // define the rounds
  int rounds;
  rounds = 5;

  // we need to scale the meshes (Unreal uses 'cm', Recast uses 'm')
  constexpr float TO_METERS = 0.01f;

  int offset = 1;
  AreaType areaType;

  for (int round = 0; round < rounds; ++round)
  {
    for (UObject* SelectedObject : BP_Actors)
    {
      AActor* TempActor = Cast<AActor>(SelectedObject);
      if (!TempActor) continue;

      // check the TAG (NoExport)
      if (TempActor->ActorHasTag(FName("NoExport"))) continue;

      FString ActorName = TempActor->GetActorLabel();

      areaType = AreaType::BLOCK;
      if (ActorName.Find(TEXT("Road")) != -1)
      {
        if (ActorName.Find(TEXT("Sidewalk")) != -1)
          areaType = AreaType::SIDEWALK;
        else if (ActorName.Find(TEXT("Crosswalk")) != -1)
          areaType = AreaType::CROSSWALK;
        else if (ActorName.Find(TEXT("Grass")) != -1)
          areaType = AreaType::GRASS;
        else
          areaType = AreaType::ROAD;
      }

      if (rounds > 1)
      {
        if (areaType == AreaType::BLOCK && round != 0)
          continue;
        if (areaType == AreaType::ROAD && round != 1)
          continue;
        if (areaType == AreaType::GRASS && round != 2)
          continue;
        if (areaType == AreaType::SIDEWALK && round != 3)
          continue;
        if (areaType == AreaType::CROSSWALK && round != 4)
          continue;
      }

      f << "g " << TCHAR_TO_ANSI(*(ActorName)) << "\n";

      TArray<UActorComponent*> Components;
      TempActor->GetComponents(UStaticMeshComponent::StaticClass(), Components);

      // Check if this actor has building components to export them separately
      bool bActorHasBuildings = false;
      if (areaType == AreaType::BLOCK)
      {
        for (auto *Component : Components)
        {
          UInstancedStaticMeshComponent* comp2 = Cast<UInstancedStaticMeshComponent>(Component);
          if (IsBuilding(comp2))
          {
            bActorHasBuildings = true;
            break;
          }
        }
      }

      // If actor has buildings, write all building instances under a single object
      if (bActorHasBuildings)
      {
        f << "o " << TCHAR_TO_ANSI(*ActorName) << "_buildings\n";
        f << "usemtl block\n";  // Set material BEFORE faces

        for (auto *Component : Components)
        {
          UInstancedStaticMeshComponent* comp2 = Cast<UInstancedStaticMeshComponent>(Component);
          if (IsBuilding(comp2))
          {
            // Export bounding box for each building instance
            for (int i=0; i<comp2->GetInstanceCount(); ++i)
            {
              // get the instance transform in world space
              FTransform InstanceTransform;
              comp2->GetInstanceTransform(i, InstanceTransform, true);

              // Get the mesh bounds in local space
              FBoxSphereBounds MeshBounds = comp2->GetStaticMesh()->GetBounds();
              FVector MeshMin = MeshBounds.Origin - MeshBounds.BoxExtent;
              FVector MeshMax = MeshBounds.Origin + MeshBounds.BoxExtent;

              // Generate 8 corners of the bounding box in mesh local space
              TArray<FVector> Corners;
              Corners.Add(FVector(MeshMin.X, MeshMin.Y, MeshMin.Z));
              Corners.Add(FVector(MeshMin.X, MeshMin.Y, MeshMax.Z));
              Corners.Add(FVector(MeshMin.X, MeshMax.Y, MeshMin.Z));
              Corners.Add(FVector(MeshMin.X, MeshMax.Y, MeshMax.Z));
              Corners.Add(FVector(MeshMax.X, MeshMin.Y, MeshMin.Z));
              Corners.Add(FVector(MeshMax.X, MeshMin.Y, MeshMax.Z));
              Corners.Add(FVector(MeshMax.X, MeshMax.Y, MeshMin.Z));
              Corners.Add(FVector(MeshMax.X, MeshMax.Y, MeshMax.Z));

              // Transform corners to world space and write vertices
              for (const FVector& Corner : Corners)
              {
                FVector WorldCorner = InstanceTransform.TransformPosition(Corner);
                f << "v " << std::fixed << WorldCorner.X * TO_METERS << " " << WorldCorner.Z * TO_METERS << " " << WorldCorner.Y * TO_METERS << "\n";
              }

              // Write 12 triangular faces (2 per box side)
              // Vertex order: 0=MinMinMin, 1=MinMinMax, 2=MinMaxMin, 3=MinMaxMax, 4=MaxMinMin, 5=MaxMinMax, 6=MaxMaxMin, 7=MaxMaxMax
              int32 baseOffset = offset;
              f << "f " << baseOffset+3 << " " << baseOffset+1 << " " << baseOffset+0 << "\n";
              f << "f " << baseOffset+2 << " " << baseOffset+3 << " " << baseOffset+0 << "\n";
              f << "f " << baseOffset+7 << " " << baseOffset+3 << " " << baseOffset+2 << "\n";
              f << "f " << baseOffset+6 << " " << baseOffset+7 << " " << baseOffset+2 << "\n";
              f << "f " << baseOffset+5 << " " << baseOffset+7 << " " << baseOffset+6 << "\n";
              f << "f " << baseOffset+4 << " " << baseOffset+5 << " " << baseOffset+6 << "\n";
              f << "f " << baseOffset+1 << " " << baseOffset+5 << " " << baseOffset+4 << "\n";
              f << "f " << baseOffset+0 << " " << baseOffset+1 << " " << baseOffset+4 << "\n";
              f << "f " << baseOffset+4 << " " << baseOffset+6 << " " << baseOffset+2 << "\n";
              f << "f " << baseOffset+0 << " " << baseOffset+4 << " " << baseOffset+2 << "\n";
              f << "f " << baseOffset+1 << " " << baseOffset+3 << " " << baseOffset+7 << "\n";
              f << "f " << baseOffset+5 << " " << baseOffset+1 << " " << baseOffset+7 << "\n";

              offset += 8;
            }
          }
        }
      }

      // Export non-building components
      for (auto *Component : Components)
      {

        // check if is an instanced static mesh
        UInstancedStaticMeshComponent* comp2 = Cast<UInstancedStaticMeshComponent>(Component);
        if (comp2)
        {
          UBodySetup *body = comp2->GetBodySetup();
          if (!body) continue;

          // Check if this is a building mesh by examining the asset path
          bool bIsBuilding = false;
          if (comp2->GetStaticMesh())
          {
            FString MeshPath = comp2->GetStaticMesh()->GetPathName();
            bIsBuilding = MeshPath.Contains(TEXT("/Building/")) || MeshPath.Contains(TEXT("/Buildings/"));
          }

          // Skip buildings (already exported above)
          if (bIsBuilding)
          {
            continue;
          }

          // Export non-building instances
          for (int i=0; i<comp2->GetInstanceCount(); ++i)
          {
            FString ObjectName = ActorName +"_"+FString::FromInt(i);

            // get the instance transform in world space
            FTransform InstanceTransform;
            comp2->GetInstanceTransform(i, InstanceTransform, true);

            // Export collision geometry
            offset += WriteObjectGeom(f, ObjectName, body, InstanceTransform, areaType, offset);
          }
        }
        else
        {
          // try as static mesh
          UStaticMeshComponent* comp = Cast<UStaticMeshComponent>(Component);
          if (!comp) continue;

          UBodySetup *body = comp->GetBodySetup();
          if (!body)
            continue;

          // f << "o " << TCHAR_TO_ANSI(*(comp->GetName())) << "\n";
          FString ObjectName = ActorName +"_"+comp->GetName();

          // get the component position and transform
          FTransform CompTransform = comp->GetComponentTransform();
          FVector CompLocation = CompTransform.GetTranslation();

          offset += WriteObjectGeom(f, ObjectName, body, CompTransform, areaType, offset);
        }
      }
    }
  }
  f.close();
}

int32 FCarlaExporterModule::WriteObjectGeom(std::ofstream &f, FString ObjectName, UBodySetup *body, FTransform &CompTransform, AreaType Area, int32 Offset)
{
// @CARLAUE
  if (!body) return 0;

  constexpr float TO_METERS = 0.01f;
  FVector CompLocation = CompTransform.GetTranslation();
  int TotalVerticesAdded = 0;
  bool Written = false;

  // Check collision complexity setting
  // CTF_UseDefault/CTF_UseSimpleAsComplex: Use simple collision only
  // CTF_UseComplexAsSimple: Use complex collision only
  bool bUseSimpleCollision = (body->CollisionTraceFlag == ECollisionTraceFlag::CTF_UseDefault ||
                               body->CollisionTraceFlag == ECollisionTraceFlag::CTF_UseSimpleAsComplex ||
                               body->CollisionTraceFlag == ECollisionTraceFlag::CTF_UseSimpleAndComplex);

  bool bUseComplexCollision = (body->CollisionTraceFlag == ECollisionTraceFlag::CTF_UseComplexAsSimple ||
                                body->CollisionTraceFlag == ECollisionTraceFlag::CTF_UseSimpleAndComplex);

  // Export simple collision (boxes, spheres, capsules, convex) if enabled
  if (bUseSimpleCollision)
  {
  // try to write the box collision if any
  for (const auto &box: body->AggGeom.BoxElems)
  {
    // get data
    const int32 nbVerts = 8;
    TArray<FVector> boxVerts;
    TArray<int32> indexBuffer;

    FVector HalfExtent(box.X / 2.0f, box.Y / 2.0f, box.Z / 2.0f);

    f << "o " << TCHAR_TO_ANSI(*(ObjectName +"_box")) << "\n";

    // define the 8 vertices
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(-HalfExtent.X, -HalfExtent.Y, -HalfExtent.Z)));
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(-HalfExtent.X, -HalfExtent.Y, +HalfExtent.Z)));
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(-HalfExtent.X, +HalfExtent.Y, -HalfExtent.Z)));
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(-HalfExtent.X, +HalfExtent.Y, +HalfExtent.Z)));
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(+HalfExtent.X, -HalfExtent.Y, -HalfExtent.Z)));
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(+HalfExtent.X, -HalfExtent.Y, +HalfExtent.Z)));
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(+HalfExtent.X, +HalfExtent.Y, -HalfExtent.Z)));
    boxVerts.Add(box.Center + box.Rotation.RotateVector(FVector(+HalfExtent.X, +HalfExtent.Y, +HalfExtent.Z)));

    // define the 12 faces (36 indices)
    indexBuffer.Add(0); indexBuffer.Add(1); indexBuffer.Add(3);
    indexBuffer.Add(0); indexBuffer.Add(3); indexBuffer.Add(2);
    indexBuffer.Add(2); indexBuffer.Add(3); indexBuffer.Add(7);
    indexBuffer.Add(2); indexBuffer.Add(7); indexBuffer.Add(6);
    indexBuffer.Add(6); indexBuffer.Add(7); indexBuffer.Add(5);
    indexBuffer.Add(6); indexBuffer.Add(5); indexBuffer.Add(4);
    indexBuffer.Add(4); indexBuffer.Add(5); indexBuffer.Add(1);
    indexBuffer.Add(4); indexBuffer.Add(1); indexBuffer.Add(0);
    indexBuffer.Add(2); indexBuffer.Add(6); indexBuffer.Add(4);
    indexBuffer.Add(2); indexBuffer.Add(4); indexBuffer.Add(0);
    indexBuffer.Add(7); indexBuffer.Add(3); indexBuffer.Add(1);
    indexBuffer.Add(7); indexBuffer.Add(1); indexBuffer.Add(5);

    // write all vertex
    for (int32 j=0; j<nbVerts; j++)
    {
      const FVector &v = boxVerts[j];
      FVector vec(v.X, v.Y, v.Z);
      FVector vec3 = CompTransform.TransformVector(vec);
      FVector world(CompLocation.X + vec3.X, CompLocation.Y + vec3.Y, CompLocation.Z + vec3.Z);

      f << "v " << std::fixed << world.X * TO_METERS << " " << world.Z * TO_METERS << " " << world.Y * TO_METERS << "\n";
    }

    // set the material in function of the area type
    switch (Area)
    {
      case AreaType::ROAD:      f << "usemtl road"      << "\n"; break;
      case AreaType::GRASS:     f << "usemtl grass"     << "\n"; break;
      case AreaType::SIDEWALK:  f << "usemtl sidewalk"  << "\n"; break;
      case AreaType::CROSSWALK: f << "usemtl crosswalk" << "\n"; break;
      case AreaType::BLOCK:     f << "usemtl block"     << "\n"; break;
    }

    // write all faces
    int k = 0;
    for (int32 i=0; i<indexBuffer.Num()/3; ++i)
    {
      // inverse order for left hand
      f << "f " << Offset + indexBuffer[k+2] << " " << Offset + indexBuffer[k+1] << " " << Offset + indexBuffer[k] << "\n";
      k += 3;
    }
    TotalVerticesAdded += nbVerts;
    Offset += nbVerts;
    Written = true;
  }

  for (const auto &convex : body->AggGeom.ConvexElems)
  {
    // Use the vertex data directly from ConvexElem
    const TArray<FVector>& convexVerts = convex.VertexData;
    if (convexVerts.Num() == 0) continue;

    int32 nbVerts = convexVerts.Num();
    f << "o " << TCHAR_TO_ANSI(*(ObjectName +"_convex")) << "\n";

    // write all vertex
    for (int32 j=0; j<nbVerts; j++)
    {
      const FVector &v = convexVerts[j];
      FVector vec3 = CompTransform.TransformVector(v);
      FVector world(CompLocation.X + vec3.X, CompLocation.Y + vec3.Y, CompLocation.Z + vec3.Z);

      f << "v " << std::fixed << world.X * TO_METERS << " " << world.Z * TO_METERS << " " << world.Y * TO_METERS << "\n";
    }
    // set the material in function of the area type
    switch (Area)
    {
      case AreaType::ROAD:      f << "usemtl road"      << "\n"; break;
      case AreaType::GRASS:     f << "usemtl grass"     << "\n"; break;
      case AreaType::SIDEWALK:  f << "usemtl sidewalk"  << "\n"; break;
      case AreaType::CROSSWALK: f << "usemtl crosswalk" << "\n"; break;
      case AreaType::BLOCK:     f << "usemtl block"     << "\n"; break;
    }

    // Use index data to write faces if available
    const TArray<int32>& IndexData = convex.IndexData;
    if (IndexData.Num() >= 3)
    {
      // Validate indices are within bounds
      bool bValidIndices = true;
      for (int32 Idx : IndexData)
      {
        if (Idx < 0 || Idx >= nbVerts)
        {
          bValidIndices = false;
          break;
        }
      }

      if (bValidIndices)
      {
        for (int32 i = 0; i < IndexData.Num(); i += 3)
        {
          if (i + 2 < IndexData.Num())
          {
            // inverse order for left hand
            f << "f " << Offset + IndexData[i+2] << " " << Offset + IndexData[i+1] << " " << Offset + IndexData[i] << "\n";
          }
        }
      }
    }

    TotalVerticesAdded += nbVerts;
    Offset += nbVerts;
    Written = true;
  }
  } // End simple collision export

  // Export complex trimesh collision data from Chaos if enabled
  if (bUseComplexCollision && body->TriMeshGeometries.Num() > 0)
  {
    for (const auto& TriMeshGeometry : body->TriMeshGeometries)
    {
      if (!TriMeshGeometry) continue;

      const auto& Particles = TriMeshGeometry->Particles();
      const auto& Elements = TriMeshGeometry->Elements();

      int32 NumTriangles = Elements.GetNumTriangles();
      if (NumTriangles == 0) continue;

      f << "o " << TCHAR_TO_ANSI(*(ObjectName +"_trimesh")) << "\n";

      // Determine if using large or small indices
      bool bUsesLargeIndices = Elements.RequiresLargeIndices();
      int32 NumVertices = Particles.Size();

      // Write all vertices
      for (int32 i = 0; i < NumVertices; i++)
      {
        const auto& v = Particles.X(i);
        FVector vec(v.X, v.Y, v.Z);
        FVector vec3 = CompTransform.TransformVector(vec);
        FVector world(CompLocation.X + vec3.X, CompLocation.Y + vec3.Y, CompLocation.Z + vec3.Z);

        f << "v " << std::fixed << world.X * TO_METERS << " " << world.Z * TO_METERS << " " << world.Y * TO_METERS << "\n";
      }

      // set the material in function of the area type
      switch (Area)
      {
        case AreaType::ROAD:      f << "usemtl road"      << "\n"; break;
        case AreaType::GRASS:     f << "usemtl grass"     << "\n"; break;
        case AreaType::SIDEWALK:  f << "usemtl sidewalk"  << "\n"; break;
        case AreaType::CROSSWALK: f << "usemtl crosswalk" << "\n"; break;
        case AreaType::BLOCK:     f << "usemtl block"     << "\n"; break;
      }

      // Write all faces
      if (bUsesLargeIndices)
      {
        const auto& IndexBuffer = Elements.GetLargeIndexBuffer();
        for (int32 i = 0; i < NumTriangles; i++)
        {
          const auto& Triangle = IndexBuffer[i];
          // inverse order for left hand
          f << "f " << Offset + Triangle[2] << " " << Offset + Triangle[1] << " " << Offset + Triangle[0] << "\n";
        }
      }
      else
      {
        const auto& IndexBuffer = Elements.GetSmallIndexBuffer();
        for (int32 i = 0; i < NumTriangles; i++)
        {
          const auto& Triangle = IndexBuffer[i];
          // inverse order for left hand
          f << "f " << Offset + Triangle[2] << " " << Offset + Triangle[1] << " " << Offset + Triangle[0] << "\n";
        }
      }

      TotalVerticesAdded += NumVertices;
      Offset += NumVertices;
      Written = true;
    }
  }

  return TotalVerticesAdded;
}

void FCarlaExporterModule::AddMenuExtension(FMenuBuilder& Builder)
{
  Builder.AddMenuEntry(FCarlaExporterCommands::Get().PluginActionExportAll);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCarlaExporterModule, CarlaExporter)
