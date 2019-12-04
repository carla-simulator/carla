// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CarlaExporter.h"
#include "CarlaExporterStyle.h"
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

void FCarlaExporterModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FCarlaExporterStyle::Initialize();
	FCarlaExporterStyle::ReloadTextures();

	FCarlaExporterCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FCarlaExporterCommands::Get().PluginActionExportAsBlock,
		FExecuteAction::CreateRaw(this, &FCarlaExporterModule::PluginButtonClickedBlock),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FCarlaExporterCommands::Get().PluginActionExportAsCross,
		FExecuteAction::CreateRaw(this, &FCarlaExporterModule::PluginButtonClickedCross),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FCarlaExporterCommands::Get().PluginActionExportAsSide,
		FExecuteAction::CreateRaw(this, &FCarlaExporterModule::PluginButtonClickedSide),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FCarlaExporterCommands::Get().PluginActionExportAsRoad,
		FExecuteAction::CreateRaw(this, &FCarlaExporterModule::PluginButtonClickedRoad),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FCarlaExporterCommands::Get().PluginActionExportAll,
		FExecuteAction::CreateRaw(this, &FCarlaExporterModule::PluginButtonClickedAll),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FCarlaExporterModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FCarlaExporterModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FCarlaExporterModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FCarlaExporterStyle::Shutdown();

	FCarlaExporterCommands::Unregister();
}

void FCarlaExporterModule::PluginButtonClickedBlock()
{
	PluginButtonClicked("block");
}
void FCarlaExporterModule::PluginButtonClickedRoad()
{
	PluginButtonClicked("road");
}
void FCarlaExporterModule::PluginButtonClickedSide()
{
	PluginButtonClicked("sidewalk");
}
void FCarlaExporterModule::PluginButtonClickedCross()
{
	PluginButtonClicked("crosswalk");
}
void FCarlaExporterModule::PluginButtonClickedAll()
{
	PluginButtonClicked("all");
}
void FCarlaExporterModule::PluginButtonClicked(std::string type)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	// get all selected objects (if any)
	TArray<UObject*> BP_Actors;
	USelection* CurrentSelection = GEditor->GetSelectedActors();
	int32 SelectionNum = CurrentSelection->GetSelectedObjects(AActor::StaticClass(), BP_Actors);
	bool useNomenclature = (type == "all");

	// if no selection, then exit if we need selection
	if (SelectionNum == 0 && type != "all")
		return;

	// if no selection, then get all objects
	if (SelectionNum == 0)
	{
		// for (TObjectIterator<UStaticMeshComponent> it; it; ++it)
		for (TActorIterator<AActor> it(World); it; ++it)
				BP_Actors.Add(Cast<UObject>(*it));
	}

	// get target path
	FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());
	// build final name
	std::ostringstream name;
	name << TCHAR_TO_UTF8(*Path) << "/" << TCHAR_TO_UTF8(*World->GetMapName()) << "_" << type << ".obj";
	// create the file
	std::ofstream f(name.str());

	int offset = 1;
	for (UObject* SelectedObject : BP_Actors)
	{
		AActor* TempActor = Cast<AActor>(SelectedObject);
		if (!TempActor) continue;
		
		// check the TAG (NoExport)
		if (TempActor->ActorHasTag(FName("NoExport"))) continue;

		FString ActorName = TempActor->GetName();

		f << "o " << TCHAR_TO_ANSI(*(ActorName)) << "\n";

		// check type by nomenclature
		if (ActorName.StartsWith("Road_Road"))
			type = "road";
		else if (ActorName.StartsWith("Road_Marking"))
			type = "road";
		else if (ActorName.StartsWith("Road_Curb"))
			type = "road";
		else if (ActorName.StartsWith("Road_Gutter"))
			type = "road";
		else if (ActorName.StartsWith("Road_Sidewalk"))
			type = "sidewalk";
		else if (ActorName.StartsWith("Road_Crosswalk"))
			type = "crosswalk";
		else if (ActorName.StartsWith("Road_Grass"))
			type = "grass";
		else
			type = "block";

		TArray<UActorComponent*> Components = TempActor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
		// UE_LOG(LogTemp, Warning, TEXT("Components: %d"), Components.Num());
		for (auto *Component : Components)
		{

			// check if is an instanced static mesh
			UInstancedStaticMeshComponent* comp2 = Cast<UInstancedStaticMeshComponent>(Component);
			if (comp2)
			{
				UBodySetup *body = comp2->GetBodySetup();
				if (!body) continue;
				// UE_LOG(LogTemp, Warning, TEXT("Get %d component instances"), comp2->GetInstanceCount());

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

							f << "v " << std::fixed << (InstanceLocation.X + world.X) * 0.01f << " " << (InstanceLocation.Z + world.Z) * 0.01f << " " << (InstanceLocation.Y + world.Y) * 0.01f << "\n";
						}
						// write all faces
						f << "usemtl " << type << "\n";
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

						f << "v " << std::fixed << world.X * 0.01f << " " << world.Z * 0.01f << " " << world.Y * 0.01f << "\n";
						// f << "v " << std::fixed << world.X << " " << world.Z << " " << world.Y << "\n";
					}
					// write all faces
					f << "usemtl " << type << "\n";
					int k = 0;
					for (PxU32 i=0; i<mesh->getNbTriangles(); ++i)
					{
						// f << "f " << offset + indexBuffer[k] << " " << offset + indexBuffer[k+1] << " " << offset + indexBuffer[k+2] << "\n";
						// inverse order for left hand
						f << "f " << offset + indexBuffer[k+2] << " " << offset + indexBuffer[k+1] << " " << offset + indexBuffer[k] << "\n";
						k += 3;
					}
					offset += nbVerts;
				}
			}
		}
	}
	f.close();
}

void FCarlaExporterModule::AddMenuExtension(FMenuBuilder& Builder)
{
	//Builder.AddMenuEntry(FCarlaExporterCommands::Get().PluginAction);
	Builder.AddMenuEntry(FCarlaExporterCommands::Get().PluginActionExportAsBlock);
	Builder.AddMenuEntry(FCarlaExporterCommands::Get().PluginActionExportAsRoad);
	Builder.AddMenuEntry(FCarlaExporterCommands::Get().PluginActionExportAsSide);
	Builder.AddMenuEntry(FCarlaExporterCommands::Get().PluginActionExportAsCross);
	Builder.AddMenuEntry(FCarlaExporterCommands::Get().PluginActionExportAll);
}

void FCarlaExporterModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	//Builder.AddToolBarButton(FCarlaExporterCommands::Get().PluginAction);
	// Builder.AddToolBarButton(FCarlaExporterCommands::Get().PluginActionExportAsBlock);
	// Builder.AddToolBarButton(FCarlaExporterCommands::Get().PluginActionExportAsRoad);
	// Builder.AddToolBarButton(FCarlaExporterCommands::Get().PluginActionExportAsSide);
	// Builder.AddToolBarButton(FCarlaExporterCommands::Get().PluginActionExportAsCross);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCarlaExporterModule, CarlaExporter)
