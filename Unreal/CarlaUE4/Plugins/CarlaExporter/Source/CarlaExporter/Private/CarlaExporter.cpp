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

#include <fstream>
#include <sstream>

static const FName CarlaExporterTabName("CarlaExporter");

#define LOCTEXT_NAMESPACE "FCarlaExporterModule"

enum AreaType
{
  ROAD,
  GRASS,
  SIDEWALK,
  CROSSWALK,
  BLOCK
};

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
      if (ActorName.StartsWith("Road_Road"))
        areaType = AreaType::ROAD;
      else if (ActorName.StartsWith("Road_Marking"))
        areaType = AreaType::ROAD;
      else if (ActorName.StartsWith("Road_Curb"))
        areaType = AreaType::ROAD;
      else if (ActorName.StartsWith("Road_Gutter"))
        areaType = AreaType::ROAD;
      else if (ActorName.StartsWith("Road_Sidewalk"))
        areaType = AreaType::SIDEWALK;
      else if (ActorName.StartsWith("Road_Crosswalk"))
        areaType = AreaType::CROSSWALK;
      else if (ActorName.StartsWith("Road_Grass"))
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

      f << "o " << TCHAR_TO_ANSI(*(ActorName)) << "\n";

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
            f << "g instance_" << i << "\n";

            // get the component position and transform
            FTransform InstanceTransform;
            comp2->GetInstanceTransform(i, InstanceTransform, true);
            FVector InstanceLocation = InstanceTransform.GetTranslation();


            // through all convex elements
            for (const auto &mesh : body->TriMeshes)
            {
              // get data
              PxU32 nbVerts = mesh->getNbVertices();
              const PxVec3* convexVerts = mesh->getVertices();
              const PxU16* indexBuffer = (PxU16 *) mesh->getTriangles();

              // write all vertex
              for(PxU32 j=0;j<nbVerts;j++)
              {
                const PxVec3 &v = convexVerts[j];
                FVector vec(v.x, v.y, v.z);
                FVector vec3 = InstanceTransform.TransformVector(vec);
                FVector world(vec3.X, vec3.Y, vec3.Z);

                f << "v " << std::fixed << (InstanceLocation.X + world.X) * TO_METERS << " " << (InstanceLocation.Z + world.Z) * TO_METERS << " " << (InstanceLocation.Y + world.Y) * TO_METERS << "\n";
              }
              // set the material in function of the area type
              switch (areaType)
              {
                case AreaType::ROAD:      f << "usemtl road"      << "\n"; break;
                case AreaType::GRASS:     f << "usemtl grass"     << "\n"; break;
                case AreaType::SIDEWALK:  f << "usemtl sidewalk"  << "\n"; break;
                case AreaType::CROSSWALK: f << "usemtl crosswalk" << "\n"; break;
                case AreaType::BLOCK:     f << "usemtl block"     << "\n"; break;
              }
              // write all faces
              for (unsigned int k=0; k<mesh->getNbTriangles()*3;k+=3)
              {
                // inverse order for left hand
                f << "f " << offset + indexBuffer[k+2] << " " << offset + indexBuffer[k+1] << " " << offset + indexBuffer[k] << "\n";
              }
              offset += nbVerts;
            }
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

          f << "g " << TCHAR_TO_ANSI(*(comp->GetName())) << "\n";

          // get the component position and transform
          FTransform CompTransform = comp->GetComponentTransform();
          FVector CompLocation = CompTransform.GetTranslation();

          // through all convex elements
          for (const auto &mesh : body->TriMeshes)
          {
            // get data
            PxU32 nbVerts = mesh->getNbVertices();
            const PxVec3* convexVerts = mesh->getVertices();
            const PxU16* indexBuffer = (PxU16 *) mesh->getTriangles();

            // write all vertex
            for(PxU32 j=0;j<nbVerts;j++)
            {
              const PxVec3 &v = convexVerts[j];
              FVector vec(v.x, v.y, v.z);
              FVector vec3 = CompTransform.TransformVector(vec);
              FVector world(CompLocation.X + vec3.X, CompLocation.Y + vec3.Y, CompLocation.Z + vec3.Z);

              f << "v " << std::fixed << world.X * TO_METERS << " " << world.Z * TO_METERS << " " << world.Y * TO_METERS << "\n";
            }
            // set the material in function of the area type
            switch (areaType)
            {
              case AreaType::ROAD:      f << "usemtl road"      << "\n"; break;
              case AreaType::GRASS:     f << "usemtl grass"     << "\n"; break;
              case AreaType::SIDEWALK:  f << "usemtl sidewalk"  << "\n"; break;
              case AreaType::CROSSWALK: f << "usemtl crosswalk" << "\n"; break;
              case AreaType::BLOCK:     f << "usemtl block"     << "\n"; break;
            }
            // write all faces
            int k = 0;
            for (PxU32 i=0; i<mesh->getNbTriangles(); ++i)
            {
              // inverse order for left hand
              f << "f " << offset + indexBuffer[k+2] << " " << offset + indexBuffer[k+1] << " " << offset + indexBuffer[k] << "\n";
              k += 3;
            }
            offset += nbVerts;
          }
        }
      }
    }
  }
  f.close();
}

void FCarlaExporterModule::AddMenuExtension(FMenuBuilder& Builder)
{
  Builder.AddMenuEntry(FCarlaExporterCommands::Get().PluginActionExportAll);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCarlaExporterModule, CarlaExporter)
