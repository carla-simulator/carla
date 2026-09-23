// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Generation/SignGenerationController.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "AssetRegistry/AssetRegistryHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "StaticMeshResources.h"
#include "GenerationPathsHelper.h"
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/rpc/String.h>
#include <carla/road/element/Waypoint.h>
#include <optional>
#include "OpenDriveToMap.h"
#include "DrawDebugHelpers.h"
#include "Carla/BlueprintLibary/MapGenFunctionLibrary.h"
#include "Carla/Traffic/GeoTrafficSign.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/MeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogSignGeneration, Log, All);

namespace
{

FString LevelShortName(const UWorld* World)
{
	return FPackageName::GetShortName(World->GetMapName());
}

/// Package folder of the persistent level, e.g. /Game/Carla/Maps/Twins/EixampleDemo.
FString LevelPackageFolder(const UWorld* World)
{
	if (World->PersistentLevel && World->PersistentLevel->GetOutermost())
	{
		return FPackageName::GetLongPackagePath(World->PersistentLevel->GetOutermost()->GetName());
	}
	return FString();
}

}

void ASignGenerationController::GetSteetMapFile()
{
	if (StreetMapData != nullptr)
	{
		return;
	}
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	const FString LevelName = LevelShortName(GetWorld());
	TArray<FString> Candidates;
	const FString LevelFolder = LevelPackageFolder(GetWorld());
	if (!LevelFolder.IsEmpty())
	{
		Candidates.Add(LevelFolder);
	}
	Candidates.Add("/Game/Carla/Maps/Twins/" + LevelName);
	// legacy generated-plugin layout
	Candidates.Add("/" + LevelName + "/Maps/OpenDrive");
	Candidates.Add("/" + LevelName + "/Content/Maps/OpenDrive");
	Candidates.Add("/" + LevelName);

	const FTopLevelAssetPath StreetMapClass = UStreetMap::StaticClass()->GetClassPathName();
	for (const FString& Folder : Candidates)
	{
		AssetRegistry.ScanPathsSynchronous({Folder}, true);
		TArray<FAssetData> Assets;
		AssetRegistry.GetAssetsByPath(FName(*Folder), Assets, true);
		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetClassPath != StreetMapClass)
			{
				continue;
			}
			StreetMapData = Cast<UStreetMap>(Asset.GetAsset());
			if (StreetMapData != nullptr)
			{
				UE_LOG(LogSignGeneration, Log, TEXT("StreetMap %s (%d sign nodes)"),
					*Asset.GetObjectPathString(), StreetMapData->GetSigns().Num());
				return;
			}
		}
	}
	UE_LOG(LogSignGeneration, Error, TEXT("No UStreetMap asset found for level %s (looked in %s)"),
		*LevelName, *FString::Join(Candidates, TEXT(", ")));
}

FString ASignGenerationController::ResolveXodrPath() const
{
	if (!XodrPath.IsEmpty())
	{
		return XodrPath;
	}
	return UOpenDriveToMap::ResolveLevelXodrPath(LevelShortName(GetWorld()));
}

bool ASignGenerationController::ResolveInputs()
{
	GetSteetMapFile();
	if (XodrPath.IsEmpty())
	{
		XodrPath = ResolveXodrPath();
	}
	if (XodrPath.IsEmpty())
	{
		UE_LOG(LogSignGeneration, Error, TEXT("No OpenDRIVE file found for level %s"), *LevelShortName(GetWorld()));
	}
	return StreetMapData != nullptr && !XodrPath.IsEmpty();
}

ASignGenerationController::ASignGenerationController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	MaxDisplacementIterations = 20;
	RoadBorderPadding = 50.0f;
	StepPercentOfLaneWidth = 0.33f;
	bDisplaceSignsToEdge = true;
	has_spawned_sign = false;
	StreetMapData = nullptr;
	current_data_asset = nullptr;
	// calibrated on EixampleDemo captures (TwinModel/ue/place_traffic_signs.py)
	PlateOffsetCm = FVector2D(0.0f, 9.0f);
	PlateHeightCm = 230.0f;
	PlateScaleByMesh.Add(TEXT("SM_OctogonalShape"), 1.6f);
	PlateScaleByMesh.Add(TEXT("SM_CircleShape"), 0.75f);
	PlateScaleByMesh.Add(TEXT("SM_InvertedTriangleShape"), 0.85f);
	PlateScaleByMesh.Add(TEXT("SM_DangerSignShape"), 0.8f);
	PlateScaleByMesh.Add(TEXT("SM_RomboidShape"), 0.75f);
	LastSignNodeCount = 0;
	LastMatchedCount = 0;
	LastSpawnedCount = 0;
}

void ASignGenerationController::BeginPlay()
{
	Super::BeginPlay();
}

void ASignGenerationController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

AGeoTrafficSign* ASignGenerationController::SpawnSign(const FVector& Location, USignDataAsset* SignAsset,
	UPoleDataAsset* PoleAsset, const FString& Label, const FString& StyleName)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.bNoFail = true;
	AGeoTrafficSign* Sign = GetWorld()->SpawnActor<AGeoTrafficSign>(Location, FRotator::ZeroRotator, Params);
	if (Sign == nullptr)
	{
		return nullptr;
	}
	Sign->SetActorLabel(Label);
	Sign->SetFolderPath(FName(TEXT("TrafficSigns")));
	Sign->SignName = SignAsset->SignName.IsEmpty()
		? UKismetSystemLibrary::GetDisplayName(SignAsset).RightChop(3) // "DA_"
		: SignAsset->SignName;
	Sign->Style = StyleName;
	Sign->ConfigureForSignal(SignAsset->XodrType, SignAsset->XodrSubtype);
	// the runtime looks baked signs up with GetAllActorsOfClass: keep them always loaded
	Sign->SetIsSpatiallyLoaded(false);
	Sign->Pole->ComponentTags.Add(FName("pole"));
	Sign->Plate->ComponentTags.Add(FName("sign"));

	if (SignAsset->SignMesh == nullptr || PoleAsset == nullptr || PoleAsset->PoleMesh == nullptr)
	{
		UE_LOG(LogSignGeneration, Warning, TEXT("Sign %s: missing plate mesh or no pole DataAsset with a mesh under %s"),
			*UKismetSystemLibrary::GetDisplayName(SignAsset), *PolePackagePath.ToString());
		return Sign;
	}

	// The catalog ships a constant material instance per sign; fall back to a dynamic
	// instance of the atlas selector for hand-made assets.
	UMaterialInterface* Material = SignAsset->Material;
	if (Material == nullptr)
	{
		UMaterialInterface* BaseMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
			TEXT("/CarlaDigitalTwinsTool/Carla/Static/Signs/Materials/Atlas/MI_SignTextureAtlasSelector")));
		if (BaseMaterial != nullptr)
		{
			UMaterialInstanceDynamic* Dynamic = UMaterialInstanceDynamic::Create(BaseMaterial, Sign);
			Dynamic->SetTextureParameterValue("Diffuse", SignAsset->Diffuse);
			// 1-based atlas cell (column, row), see USignDataAsset
			Dynamic->SetScalarParameterValue("Index_X", SignAsset->Id_X);
			Dynamic->SetScalarParameterValue("Index_Y", SignAsset->Id_Y);
			Material = Dynamic;
		}
	}

	const float* ScalePtr = PlateScaleByMesh.Find(SignAsset->SignMesh->GetName());
	const float Scale = ScalePtr ? *ScalePtr : 1.0f;
	const FBoxSphereBounds PoleBounds = PoleAsset->PoleMesh->GetBounds();
	const FBoxSphereBounds PlateBounds = SignAsset->SignMesh->GetBounds();
	const float PoleTop = PoleBounds.Origin.Z + PoleBounds.BoxExtent.Z;

	if (PoleAsset->PoleMesh->FindSocket(FName(TEXT("Sign1"))) != nullptr)
	{
		// Pole with a mounting socket: the plate takes the socket transform.
		Sign->Setup(PoleAsset->PoleMesh, SignAsset->SignMesh, Material, FVector::ZeroVector, 0.0f, Scale);
		const FTransform Socket = Sign->Pole->GetSocketTransform(FName(TEXT("Sign1")), RTS_Actor);
		Sign->Plate->SetRelativeLocation(Socket.GetLocation() + FVector(PlateOffsetCm.X, PlateOffsetCm.Y, 0.0f));
		Sign->Plate->SetRelativeRotation(Socket.GetRotation());
	}
	else
	{
		// No socket (none of the shipped poles has one): plate centre at PlateHeightCm, pole
		// shortened so its top sits just above the plate.
		Sign->Setup(PoleAsset->PoleMesh, SignAsset->SignMesh, Material,
			FVector(PlateOffsetCm.X, PlateOffsetCm.Y, PlateHeightCm), 0.0f, Scale);
		const float HalfH = PlateBounds.BoxExtent.Z * Scale;
		const float Want = PlateHeightCm + HalfH + 8.0f;
		const float PoleScaleZ = PoleTop > 0.0f ? FMath::Min(1.0f, Want / PoleTop) : 1.0f;
		Sign->Pole->SetRelativeScale3D(FVector(1.0f, 1.0f, PoleScaleZ));
	}
	return Sign;
}

void ASignGenerationController::SignGenerationByPath(FName sign_package_path, FName pole_package_path, ESignStyle sign_style)
{
	LastSignNodeCount = LastMatchedCount = LastSpawnedCount = 0;
	if (!ResolveInputs())
	{
		return;
	}

	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *XodrPath))
	{
		UE_LOG(LogSignGeneration, Error, TEXT("Cannot read OpenDRIVE file %s"), *XodrPath);
		return;
	}
	std::string opendrive_xml = carla::rpc::FromLongFString(FileContent);
	std::optional<carla::road::Map> current_carla_map = carla::opendrive::OpenDriveParser::Load(opendrive_xml);
	if (!current_carla_map.has_value())
	{
		UE_LOG(LogSignGeneration, Error, TEXT("OpenDRIVE file %s did not parse"), *XodrPath);
		return;
	}

	// A re-run replaces the previously generated signs.
	for (AActor* entry : GeneratedSigns)
	{
		if (entry != nullptr) entry->Destroy();
	}
	GeneratedSigns.Empty();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FString EnumName = UEnum::GetValueAsString(sign_style);
	int32 SeparatorIndex;
	if (EnumName.FindChar(':', SeparatorIndex))
	{
		EnumName = EnumName.Mid(SeparatorIndex + 2);
	}
	const FString FullPath = sign_package_path.ToString() + TEXT("/") + EnumName;
	const FName FullPathName = FName(*FullPath);
	PolePackagePath = pole_package_path;
	SignPackagePath = sign_package_path;
	SignStyle = sign_style;

	AssetRegistry.ScanPathsSynchronous({FullPath, pole_package_path.ToString()}, true);
	TArray<FAssetData> temp_sign;
	AssetRegistry.GetAssetsByPath(FullPathName, temp_sign, true);
	TArray<FAssetData> temp_pole;
	AssetRegistry.GetAssetsByPath(PolePackagePath, temp_pole, true);

	TArray<USignDataAsset*> sign_data;
	for (const FAssetData& asset_data : temp_sign)
	{
		USignDataAsset* temp = Cast<USignDataAsset>(asset_data.GetAsset());
		if (temp != nullptr) sign_data.Add(temp);
	}
	UPoleDataAsset* pole_asset = nullptr;
	for (const FAssetData& asset_data : temp_pole)
	{
		UPoleDataAsset* temp = Cast<UPoleDataAsset>(asset_data.GetAsset());
		if (temp != nullptr && temp->PoleMesh != nullptr) { pole_asset = temp; break; }
	}
	UE_LOG(LogSignGeneration, Log, TEXT("%d sign DataAssets under %s, pole %s, %d OSM sign nodes"),
		sign_data.Num(), *FullPath, pole_asset ? *pole_asset->GetName() : TEXT("NONE"), StreetMapData->GetSigns().Num());

	TMap<FString, int> spawn_name_counters;
	LastSignNodeCount = StreetMapData->GetSigns().Num();

	for (const FStreetMapMisc& sign : StreetMapData->GetSigns())
	{
		has_spawned_sign = false;

		FString keyname;
		FString signValue;
		// traffic_sign=* names the sign directly; highway=stop/give_way/crossing and maxspeed=*
		// are the usual node tags (plus the way maxspeed the importer turns into a mid-way sign).
		GetSignPropertyValue(sign, "traffic_sign", keyname, signValue);
		if (signValue.IsEmpty()) GetSignPropertyValue(sign, "highway", keyname, signValue);
		if (signValue.IsEmpty()) GetSignPropertyValue(sign, "crossing", keyname, signValue);
		if (signValue.IsEmpty()) GetSignPropertyValue(sign, "maxspeed", keyname, signValue);
		if (signValue.IsEmpty()) continue;

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
			if (!bMatches) continue;

			LastMatchedCount++;
			int& counter = spawn_name_counters.FindOrAdd(signValue);
			counter++;
			FString actor_name = "TrafficSign_" + keyname + "_" + signValue + "_";
			actor_name.AppendInt(counter);

			// UStreetMap positions are centimetres east / south of the import origin
			// (UStreetMapFactory::LatLonOrigin), i.e. the level frame when the origin is the twin datum.
			const FVector position(sign.Position.X, sign.Position.Y, 0.0f);
			AGeoTrafficSign* actor = SpawnSign(position, sign_asset, pole_asset, actor_name, EnumName);
			if (actor == nullptr) continue;
			GeneratedSigns.Add(actor);
			has_spawned_sign = true;
			LastSpawnedCount++;
		}
	}

	// Push every sign out of the carriageway, drop it on the ground and turn its print to the
	// oncoming traffic of the nearest lane (actor yaw = lane heading + 90, the runtime's
	// convention for signal actors).
	const int32 check_shoulder_or_driving =
		static_cast<int32_t>(carla::road::Lane::LaneType::Shoulder) |
		static_cast<int32_t>(carla::road::Lane::LaneType::Driving);
	TArray<AActor*> Kept;
	for (AActor* sign : GeneratedSigns)
	{
		FVector sign_location = sign->GetActorLocation();
		if (bDisplaceSignsToEdge)
		{
			sign_location = UOpenDriveToMap::DisplaceLocationOutsideNeighboringRoads(current_carla_map.value(), sign_location, StepPercentOfLaneWidth, RoadBorderPadding);
		}

		std::optional<carla::road::element::Waypoint> closest_waypoint =
			current_carla_map->GetClosestWaypointOnRoad(sign_location, check_shoulder_or_driving);
		FRotator rot = sign->GetActorRotation();
		float road_z = sign_location.Z;
		if (closest_waypoint.has_value())
		{
			const carla::geom::Transform road_transform = current_carla_map->ComputeTransform(closest_waypoint.value());
			rot = FRotator(0.0f, road_transform.rotation.yaw + 90.0f, 0.0f);
			road_z = road_transform.location.z * 100.0f;
		}
		else
		{
			// nowhere near a road: not a sign the level can use
			sign->Destroy();
			LastSpawnedCount--;
			continue;
		}

		// Ground under the sign: the level's geometry when it is loaded (a line trace), the
		// OpenDRIVE elevation of the nearest lane otherwise (World Partition cells are not
		// streamed in under a commandlet).
		FHitResult hit_result;
		UKismetSystemLibrary::LineTraceSingle(Cast<UObject>(this),
			FVector(sign_location.X, sign_location.Y, road_z + 5000.0f),
			FVector(sign_location.X, sign_location.Y, road_z - 5000.0f),
			ETraceTypeQuery::TraceTypeQuery1,
			true,
			GeneratedSigns,
			EDrawDebugTrace::None,
			hit_result,
			true);
		sign_location.Z = hit_result.bBlockingHit ? hit_result.ImpactPoint.Z : road_z;
		sign->SetActorLocationAndRotation(sign_location, rot);
		Kept.Add(sign);
	}
	GeneratedSigns = Kept;

	// Signs that ended up on the same spot share one pole: their plates stack under the first one.
	TMap<FVector, TArray<AActor*>> LocationMap;
	for (AActor* Actor : GeneratedSigns)
	{
		if (!Actor) continue;
		LocationMap.FindOrAdd(Actor->GetActorLocation()).Add(Actor);
	}
	for (const auto& Elem : LocationMap)
	{
		const TArray<AActor*>& ActorsAtLocation = Elem.Value;
		if (ActorsAtLocation.Num() < 2) continue;
		AGeoTrafficSign* First = Cast<AGeoTrafficSign>(ActorsAtLocation[0]);
		if (First == nullptr || First->Plate->GetStaticMesh() == nullptr) continue;
		UE_LOG(LogSignGeneration, Log, TEXT("%d signs at %s share a pole"), ActorsAtLocation.Num(), *Elem.Key.ToString());
		const FVector FirstOffset = First->Plate->GetRelativeLocation();
		float Drop = First->Plate->GetStaticMesh()->GetBounds().BoxExtent.Z * First->Plate->GetRelativeScale3D().Z * 2.0f + 5.0f;
		for (int i = 1; i < ActorsAtLocation.Num(); i++)
		{
			AGeoTrafficSign* Other = Cast<AGeoTrafficSign>(ActorsAtLocation[i]);
			if (Other == nullptr || Other->Plate->GetStaticMesh() == nullptr) { continue; }
			UStaticMeshComponent* Extra = UMapGenFunctionLibrary::AddStaticMeshComponentToActor(First);
			Extra->SetStaticMesh(Other->Plate->GetStaticMesh());
			Extra->SetMaterial(0, Other->Plate->GetMaterial(0));
			Extra->SetRelativeScale3D(Other->Plate->GetRelativeScale3D());
			Extra->SetRelativeRotation(Other->Plate->GetRelativeRotation());
			Extra->SetRelativeLocation(FirstOffset - FVector(0.0f, 0.0f, Drop * i));
			Extra->ComponentTags.Add(FName("sign"));
			GeneratedSigns.Remove(Other);
			Other->Destroy();
			LastSpawnedCount--;
		}
	}
	UE_LOG(LogSignGeneration, Log, TEXT("sign nodes %d, matched %d, standing %d"),
		LastSignNodeCount, LastMatchedCount, LastSpawnedCount);
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
