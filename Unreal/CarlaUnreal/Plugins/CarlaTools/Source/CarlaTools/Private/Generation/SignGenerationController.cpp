// Fill out your copyright notice in the Description page of Project Settings.


#include "Generation/SignGenerationController.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "AssetRegistry/AssetRegistryHelpers.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"
#include "GenerationPathsHelper.h"
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/rpc/String.h>
#include <carla/road/element/Waypoint.h>
#include <optional>
#include "OpenDriveToMap.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Carla/BlueprintLibary/MapGenFunctionLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/MeshComponent.h"
#include "OpenDriveToMap.h"

void ASignGenerationController::GetSteetMapFile()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	FString LevelName = FPackageName::GetShortName(GetWorld()->GetMapName());
	FString StreetMapObjectPath = "/" + LevelName + "/Content/Maps/OpenDrive/";// + LevelName + ".osm";
	TArray<FAssetData> street_map_data_assets;
	AssetRegistry.GetAssetsByPath(FName(*StreetMapObjectPath), street_map_data_assets);

	for (FAssetData asset : street_map_data_assets)
	{
		StreetMapData = Cast<UStreetMap>(asset.GetAsset());
		if (StreetMapData != nullptr) break;
	}
}

// Sets default values
ASignGenerationController::ASignGenerationController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MaxDisplacementIterations = true;
	MaxDisplacementIterations = 20;
	RoadBorderPadding = 50.0f;
	StepPercentOfLaneWidth = 0.33f;
	bDisplaceSignsToEdge = true;
	has_spawned_sign = false;
}

// Called when the game starts or when spawned
void ASignGenerationController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASignGenerationController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASignGenerationController::SignGenerationByPath(FName sign_package_path, FName pole_package_path, ESignStyle sign_style)
{

	GetSteetMapFile();
	if (StreetMapData == nullptr) return;

	std::optional<carla::road::Map> current_carla_map;

	FString FileContent;
	FString LevelName = FPackageName::GetShortName(GetWorld()->GetMapName());
	FString FilePath = UGenerationPathsHelper::GetRawMapDirectoryPath(LevelName) + "OpenDrive/" + LevelName + ".xodr";
	FFileHelper::LoadFileToString(FileContent, *FilePath);
	std::string opendrive_xml = carla::rpc::FromLongFString(FileContent);
	//UE_LOG(LogCarlaTools, Warning, TEXT("UOpenDriveToMap::GenerateTile() Loading File..... "));
	current_carla_map = carla::opendrive::OpenDriveParser::Load(opendrive_xml);

	//When creating actors again, destroy all the previously created and clear the arrays.
	// 
	//for (const TPair<int32, AStaticMeshActor*> entry : GeneratedSigns)
	for (AActor* entry : GeneratedSigns)
	{
		//if(entry.Value != nullptr) entry.Value->Destroy();
		if (entry != nullptr) entry->Destroy();
	}

	GeneratedSigns.Empty();
	//------------------------------------------------------------------------------------

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FString EnumName = UEnum::GetValueAsString(sign_style);
	// Remove the "ESignStyle::" part
	int32 SeparatorIndex;
	if (EnumName.FindChar(':', SeparatorIndex))
	{
		EnumName = EnumName.Mid(SeparatorIndex + 2);
	}
	FString FullPath = sign_package_path.ToString() + TEXT("/") + EnumName;
	FName FullPathName = FName(*FullPath);

	UE_LOG(LogTemp, Warning, TEXT("SignPackagePath: %s"), *FullPathName.ToString());

	PolePackagePath = pole_package_path; //temporary

	//We get the sign from a path to the content folders
	TArray<FAssetData> temp_sign;
	AssetRegistry.GetAssetsByPath(FullPathName, temp_sign, true);
	TArray<FAssetData> temp_pole;
	AssetRegistry.GetAssetsByPath(PolePackagePath, temp_pole, true);

	TArray<USignDataAsset*> sign_data;
	for (FAssetData asset_data : temp_sign)
	{
		USignDataAsset* temp = Cast<USignDataAsset>(asset_data.GetAsset());
		if (temp != nullptr) sign_data.Add(temp);
	}
	TArray<UPoleDataAsset*> pole_data;
	for (FAssetData asset_data : temp_pole)
	{
		UPoleDataAsset* temp = Cast<UPoleDataAsset>(asset_data.GetAsset());
		if (temp != nullptr) pole_data.Add(temp);
	}

	TMap<FString, int> spawn_name_counters;

	for (FStreetMapMisc sign : StreetMapData->GetSigns())
	{
		has_spawned_sign = false;

		FString keyname;
		FString signValue;

		GetSignPropertyValue(sign, "highway", keyname, signValue);
		if (signValue.IsEmpty()) GetSignPropertyValue(sign, "crossing", keyname, signValue);
		if (signValue.IsEmpty()) GetSignPropertyValue(sign, "maxspeed", keyname, signValue);
		if (signValue.IsEmpty()) continue;

		if (!keyname.IsEmpty())
		{
			UE_LOG(LogTemp, Log, TEXT("SignValue detected is: %s"), *signValue);
		}

		for (USignDataAsset* sign_asset : sign_data)
		{
			if (has_spawned_sign) break;
			// Exact match on the catalog's OSM tags (key=value); the legacy substring match on the
			// asset name ("DA_max_speed_20" matched "20", "DA_stop" matched "bus_stop") is kept only
			// for assets that carry no tags at all, and then requires the whole name to match.
			const bool bMatches = sign_asset->OsmTags.Num() > 0
				? sign_asset->MatchesOsmTag(keyname, signValue)
				: (sign_asset->SignName.IsEmpty()
					? UKismetSystemLibrary::GetDisplayName(sign_asset).Equals(TEXT("DA_") + signValue, ESearchCase::IgnoreCase)
					: sign_asset->SignName.Equals(signValue, ESearchCase::IgnoreCase));
			if (bMatches)
			{
				int& counter = spawn_name_counters.FindOrAdd(signValue);
				counter++;

				FString actor_name = "TrafficSign_" + keyname + "_" + signValue + "_";
				actor_name.AppendInt(counter);

				AStaticMeshActor* temp_actor = GetWorld()->SpawnActor<AStaticMeshActor>();

				carla::geom::Location cl(FVector(sign.Position.X, sign.Position.Y, 0.0f));
				//wp = GetClosestWaypoint(pos). if distance wp - pos == lane_width --> estas al borde de la carretera
				//std::optional<Waypoint> wp = current_carla_map->GetClosestWaypointOnRoad(cl);
				FVector position = FVector(sign.Position.X, sign.Position.Y, 0.0f);
				//float dist = OpenDriveMap->DistanceToLaneBorder(current_carla_map, position);

				std::optional<carla::road::element::Waypoint> closest_waypoint = current_carla_map->GetClosestWaypointOnRoad(cl, (int32)carla::road::Lane::LaneType::Sidewalk);
				//GetRight / GetLeft
				//CheckSignalsOnRoads
				temp_actor->SetActorLocation(FVector(sign.Position.X, sign.Position.Y, 0.0f));


				temp_actor->SetActorLabel(actor_name);

				UPoleDataAsset* pole_asset = nullptr;
				for (UPoleDataAsset* candidate : pole_data)
				{
					if (candidate != nullptr && candidate->PoleMesh != nullptr) { pole_asset = candidate; break; }
				}
				if (sign_asset->SignMesh != nullptr && pole_asset != nullptr)
				{
					UStaticMeshComponent* pole_mesh_comp = UMapGenFunctionLibrary::AddStaticMeshComponentToActor(temp_actor);
					pole_mesh_comp->SetStaticMesh(pole_asset->PoleMesh);
					pole_mesh_comp->ComponentTags.Add(FName("pole"));

					UStaticMeshComponent* sign_mesh_comp = UMapGenFunctionLibrary::AddStaticMeshComponentToActor(temp_actor);
					sign_mesh_comp->SetStaticMesh(sign_asset->SignMesh);

					// The catalog ships a constant material instance per sign (cell baked in); fall
					// back to a dynamic instance of the atlas selector for hand-made assets.
					if (sign_asset->Material != nullptr)
					{
						sign_mesh_comp->SetMaterial(0, sign_asset->Material);
					}
					else
					{
						UMaterialInterface* BaseMaterial = Cast<UMaterialInterface>(
							StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
								TEXT("/CarlaDigitalTwinsTool/Carla/Static/Signs/Materials/Atlas/MI_SignTextureAtlasSelector"))
						);
						UMaterialInstanceDynamic* DynamicMaterial = nullptr;
						if (BaseMaterial)
						{
							DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
						}
						if (DynamicMaterial)
						{
							//Set diffuse texture from the data asset, Normal map gets created using diffuse.
							DynamicMaterial->SetTextureParameterValue("Diffuse", sign_asset->Diffuse);
							// 1-based atlas cell (column, row), see USignDataAsset
							DynamicMaterial->SetScalarParameterValue("Index_X", sign_asset->Id_X);
							DynamicMaterial->SetScalarParameterValue("Index_Y", sign_asset->Id_Y);
						}
						sign_mesh_comp->SetMaterial(0, DynamicMaterial);
					}
					// Mount at the pole's Sign1 socket; poles without one hang the plate just below
					// the pole top (GetSocketTransform would otherwise return the pole base).
					if (pole_mesh_comp->DoesSocketExist(FName(TEXT("Sign1"))))
					{
						sign_mesh_comp->SetWorldTransform(pole_mesh_comp->GetSocketTransform(FName(TEXT("Sign1"))));
					}
					else
					{
						const FBoxSphereBounds PoleBounds = pole_asset->PoleMesh->GetBounds();
						const FBoxSphereBounds PlateBounds = sign_asset->SignMesh->GetBounds();
						const float PoleTop = PoleBounds.Origin.Z + PoleBounds.BoxExtent.Z;
						sign_mesh_comp->SetRelativeLocation(FVector(0.0f, 0.0f, PoleTop - PlateBounds.BoxExtent.Z - 5.0f));
					}
					sign_mesh_comp->ComponentTags.Add(FName("sign"));
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Sign %s: missing plate mesh or no pole DataAsset with a mesh under %s"),
						*UKismetSystemLibrary::GetDisplayName(sign_asset), *PolePackagePath.ToString());
				}

				//GeneratedSigns.Add(sign.OSM_ID, temp_actor);
				GeneratedSigns.Add(temp_actor);
				has_spawned_sign = true;
			}
		}
	}

	//Moving Signals to Sidewalk
	if(bDisplaceSignsToEdge)
	{

		for (AActor* sign : GeneratedSigns)
		{
			FVector sign_location = sign->GetActorLocation();
			FRotator sign_rotation = sign->GetActorRotation();
			
			sign_location = UOpenDriveToMap::DisplaceLocationOutsideNeighboringRoads(Cast<UObject>(this), sign_location, StepPercentOfLaneWidth, RoadBorderPadding);
			sign->SetActorLocation(sign_location);

			int32 check_shoulder_or_driving =
				static_cast<int32_t>(carla::road::Lane::LaneType::Shoulder) |
				static_cast<int32_t>(carla::road::Lane::LaneType::Driving);

			std::optional<carla::road::element::Waypoint> closest_waypoint =
				current_carla_map->GetClosestWaypointOnRoad(sign_location, check_shoulder_or_driving);

			if (closest_waypoint.has_value()) 
			{
				std::vector<carla::road::element::Waypoint> succesor_waypoints = 
					current_carla_map->GetSuccessors(closest_waypoint.value());

				if (succesor_waypoints.size() > 0) 
				{
					carla::road::element::Waypoint next_waypoint = succesor_waypoints[0];

					carla::geom::Transform road_transform = current_carla_map->ComputeTransform(closest_waypoint.value());
					carla::geom::Transform road_transform_next = current_carla_map->ComputeTransform(next_waypoint);

					FRotator rot = UKismetMathLibrary::MakeRotFromX(FVector(road_transform.location - road_transform_next.location).GetSafeNormal());
					sign->SetActorRotation(rot);
				}
			}
			
			FHitResult hit_result;

			UKismetSystemLibrary::LineTraceSingle(Cast<UObject>(this),
				FVector(sign_location.X, sign_location.Y, 50000.0f),
				FVector(sign_location.X, sign_location.Y, -100000.0f),
				ETraceTypeQuery::TraceTypeQuery1,
				true,
				GeneratedSigns,
				EDrawDebugTrace::ForDuration,
				hit_result,
				true,
				FLinearColor::Yellow, FLinearColor::Green, 3.0f);

			if (!hit_result.bBlockingHit) sign->Destroy();
		}
	}

	// We get a map of the generated signs actors with the location as the key
	TMap<FVector, TArray<AActor*>> LocationMap;

	for (AActor* Actor : GeneratedSigns)
	{
		if (!Actor) continue;

		FVector Location = Actor->GetActorLocation();

		// Use Add to group actors by location
		TArray<AActor*>& ActorList = LocationMap.FindOrAdd(Location);
		ActorList.Add(Actor);
	}

	// Get the component of the signs that share location and put it in the first sign with an offset, then destroy the duplicated sign
	for (const auto& Elem : LocationMap)
	{
		const FVector& Location = Elem.Key;
		const TArray<AActor*>& ActorsAtLocation = Elem.Value;
		AActor* FirstSignActor = ActorsAtLocation[0];

		UActorComponent* PoleComp = FirstSignActor->FindComponentByTag(UStaticMeshComponent::StaticClass(), FName("pole"));
		if (PoleComp == nullptr) continue;

		UStaticMeshComponent* PoleMesh = Cast<UStaticMeshComponent>(PoleComp);
		if (PoleMesh == nullptr) continue;

		FTransform SignTransform = PoleMesh->GetSocketTransform(FName(TEXT("Sign1")));

		if (ActorsAtLocation.Num() > 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found %d actors at location %s"),
				ActorsAtLocation.Num(), *Location.ToString());

			for (int i = 0 ; i < ActorsAtLocation.Num(); i++)
			{
				if (i == 0) continue;
				AActor* CurrentActor = ActorsAtLocation[i];
				UActorComponent* SignComp = CurrentActor->FindComponentByTag(UStaticMeshComponent::StaticClass(), FName("sign"));
				UStaticMeshComponent* SignMeshComp = Cast<UStaticMeshComponent>(SignComp);

				UStaticMeshComponent* SignComponent = UMapGenFunctionLibrary::AddStaticMeshComponentToActor(FirstSignActor);

				SignComponent->SetWorldTransform(FTransform(
					SignTransform.GetRotation(), 
					FVector(SignTransform.GetLocation().X, SignTransform.GetLocation().Y, SignTransform.GetLocation().Z * i - 90),
					SignTransform.GetScale3D()));

				SignComponent->SetStaticMesh(SignMeshComp->GetStaticMesh());
				SignComponent->SetMaterial(0, SignMeshComp->GetMaterial(0));

				CurrentActor->Destroy();

			}
		}
	}
}

void ASignGenerationController::SignGenerationForCurrentMap()
{
	SignGenerationByPath(SignPackagePath, PolePackagePath, SignStyle);
}

void ASignGenerationController::GetSignPropertyValue(FStreetMapMisc Sign, FString KeyToFind, FString& Out_KeyName, FString& Out_Value)
{
	FString* FoundValue = Sign.Properties.Find(KeyToFind);

	if (FoundValue == nullptr)
	{
		Out_Value = "";
		Out_KeyName = "";
		return;
	}

	Out_Value = *FoundValue;
	Out_KeyName = KeyToFind;
}