// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
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
#include "Runtime/Engine/Classes/Engine/Selection.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConvexElem.h"
#include "PxTriangleMesh.h"
#include "PxVec3.h"
#include "LevelEditor.h"
#include "EngineUtils.h"
#include "PhysXPublic.h"
#include "PhysicsPublic.h"
#include "PhysXIncludes.h"
#include "PxSimpleTypes.h"
#include <fstream>
#include <sstream>

static const FName CarlaExporterTabName("CarlaExporter");

#define LOCTEXT_NAMESPACE "FCarlaExporterModule"

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

      FString ActorName = TempActor->GetName();

      // check type by nomenclature
      if (ActorName.Find("Road_Road") != -1 || ActorName.Find("Roads_Road") != -1)
        areaType = AreaType::ROAD;
      else if (ActorName.Find("Road_Marking") != -1 || ActorName.Find("Roads_Marking") != -1)
        areaType = AreaType::ROAD;
      else if (ActorName.Find("Road_Curb") != -1 || ActorName.Find("Roads_Curb") != -1)
        areaType = AreaType::ROAD;
      else if (ActorName.Find("Road_Gutter") != -1 || ActorName.Find("Roads_Gutter") != -1)
        areaType = AreaType::ROAD;
      else if (ActorName.Find("Road_Sidewalk") != -1 || ActorName.Find("Roads_Sidewalk") != -1)
        areaType = AreaType::SIDEWALK;
      else if (ActorName.Find("Road_Crosswalk") != -1 || ActorName.Find("Roads_Crosswalk") != -1)
        areaType = AreaType::CROSSWALK;
      else if (ActorName.Find("Road_Grass") != -1 || ActorName.Find("Roads_Grass") != -1)
        areaType = AreaType::GRASS;
      else
        areaType = AreaType::BLOCK;

      // check to export in this round or not
      if (rounds > 1)
      {
        if (areaType == AreaType::BLOCK && round != 0)
          continue;
        else if (areaType == AreaType::ROAD && round != 1)
          continue;
        else if (areaType == AreaType::GRASS && round != 2)
          continue;
        else if (areaType == AreaType::SIDEWALK && round != 3)
          continue;
        else if (areaType == AreaType::CROSSWALK && round != 4)
          continue;
      }

      f << "g " << TCHAR_TO_ANSI(*(ActorName)) << "\n";

      TArray<UActorComponent*> Components = TempActor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
      for (auto *Component : Components)
      {

        // check if is an instanced static mesh
        UInstancedStaticMeshComponent* comp2 = Cast<UInstancedStaticMeshComponent>(Component);
        if (comp2)
        {
          UBodySetup *body = comp2->GetBodySetup();
          if (!body) continue;

          for (int i=0; i<comp2->GetInstanceCount(); ++i)
          {
            // f << " instance_" << i << "\n";
            FString ObjectName = ActorName +"_"+FString::FromInt(i);

            // get the component position and transform
            FTransform InstanceTransform;
            comp2->GetInstanceTransform(i, InstanceTransform, true);
            FVector InstanceLocation = InstanceTransform.GetTranslation();

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
  if (!body) return 0;

  constexpr float TO_METERS = 0.01f;
  FVector CompLocation = CompTransform.GetTranslation();
  int TotalVerticesAdded = 0;
  bool Written = false;

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

  // try to write the simple collision mesh
  for (const auto &convex : body->AggGeom.ConvexElems)
  {
    // get data
    PxConvexMesh *mesh = convex.GetConvexMesh();
    if (!mesh) continue;
    int32 nbVerts = mesh->getNbVertices();
    const PxVec3* convexVerts = mesh->getVertices();
    const PxU8* indexBuffer = (PxU8 *) mesh->getIndexBuffer();

    f << "o " << TCHAR_TO_ANSI(*(ObjectName +"_convex")) << "\n";

    // write all vertex
    for (int32 j=0; j<nbVerts; j++)
    {
      const PxVec3 &v = convexVerts[j];
      FVector vec(v.x, v.y, v.z);
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
    for (PxU32 i=0; i<mesh->getNbPolygons(); ++i)
    {
      PxHullPolygon face;
      bool status = mesh->getPolygonData(i, face);
      const PxU8* faceIndices = indexBuffer + face.mIndexBase;
      int faceNbVerts = face.mNbVerts;
      for(int32 j=2; j<faceNbVerts; j++)
      {
        // inverse order for left hand
        f << "f " << Offset + faceIndices[j-1] << " " << Offset + faceIndices[j] << " " << Offset + faceIndices[0] << "\n";
      }
    }
    TotalVerticesAdded += nbVerts;
    Offset += nbVerts;
    Written = true;
  }

  if (!Written)
  {
    // write the mesh
    for (const auto &mesh : body->TriMeshes)
    {
      // get data
      PxU32 nbVerts = mesh->getNbVertices();
      const PxVec3* convexVerts = mesh->getVertices();

      f << "o " << TCHAR_TO_ANSI(*(ObjectName +"_mesh")) << "\n";

      // write all vertex
      for (PxU32 j=0; j<nbVerts; j++)
      {
        const PxVec3 &v = convexVerts[j];
        FVector vec(v.x, v.y, v.z);
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
      // triangle indices can be of 16 or 32 bits
      if (mesh->getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES)
      {
        PxU16 *Indices16 = (PxU16 *) mesh->getTriangles();
        for (PxU32 i=0; i<mesh->getNbTriangles(); ++i)
        {
          // inverse order for left hand
          f << "f " << Offset + Indices16[k+2] << " " << Offset + Indices16[k+1] << " " << Offset + Indices16[k] << "\n";
          k += 3;
        }
      }
      else
      {
        PxU32 *Indices32 = (PxU32 *) mesh->getTriangles();
        for (PxU32 i=0; i<mesh->getNbTriangles(); ++i)
        {
          // inverse order for left hand
          f << "f " << Offset + Indices32[k+2] << " " << Offset + Indices32[k+1] << " " << Offset + Indices32[k] << "\n";
          k += 3;
        }
      }
      TotalVerticesAdded += nbVerts;
      Offset += nbVerts;
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
