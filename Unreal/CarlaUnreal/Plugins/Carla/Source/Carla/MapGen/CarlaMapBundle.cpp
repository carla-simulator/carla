// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/MapGen/CarlaMapBundle.h"
#include "Carla.h"

#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Game/Tagger.h"
#include "Carla/OpenDrive/OpenDrive.h"
#include "Carla/OpenDrive/OpenDriveGenerator.h"
#include "Carla/Vehicle/VehicleSpawnPoint.h"

#include <util/ue-header-guard-begin.h>
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include <util/ue-header-guard-end.h>

static const TMap<FString, FString> GCarlaBundleMaterialCatalog = {
    // Roads & sidewalks
    {TEXT("road"), TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Asphalt_A.MI_Road_Asphalt_A")},
    {TEXT("road_asphalt"), TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Asphalt_A.MI_Road_Asphalt_A")},
    {TEXT("road_worn"), TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Asphalt_B.MI_Road_Asphalt_B")},
    {TEXT("road_wet"), TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Asphalt_B.MI_Road_Asphalt_B")},
    {TEXT("road_rural"), TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Rural_A.MI_Road_Rural_A")},
    {TEXT("sidewalk"), TEXT("/Game/Carla/Static/GenericMaterials/Sidewalk/MI_Sidewalk_Apartment.MI_Sidewalk_Apartment")},
    {TEXT("sidewalk_concrete"), TEXT("/Game/Carla/Static/GenericMaterials/Sidewalk/MI_Sidewalk_Apartment.MI_Sidewalk_Apartment")},
    {TEXT("sidewalk_stone"), TEXT("/Game/Carla/Static/GenericMaterials/Sidewalk/MI_sidewalk_02.MI_sidewalk_02")},
    {TEXT("sidewalk_tiles"), TEXT("/Game/Carla/Static/GenericMaterials/Sidewalk/MI_sidewalk_01.MI_sidewalk_01")},
    {TEXT("crosswalk"), TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Asphalt_B_LaneMarkingWhite.MI_Road_Asphalt_B_LaneMarkingWhite")},
    {TEXT("curb"), TEXT("/Game/Carla/Static/GenericMaterials/Gutters_Curbs/Curb/MI_CurbDirty01.MI_CurbDirty01")},

    // Buildings & architecture
    {TEXT("building"), TEXT("/Game/Carla/Static/GenericMaterials/Facade/MI_Facade01.MI_Facade01")},
    {TEXT("building_concrete"), TEXT("/Game/Carla/Static/GenericMaterials/Facade/MI_Facade01.MI_Facade01")},
    {TEXT("building_brick"), TEXT("/Game/Carla/Static/GenericMaterials/Facade/MI_Facade01.MI_Facade01")},
    {TEXT("building_glass"), TEXT("/Game/Carla/Static/GenericMaterials/Facade/MI_Facade01.MI_Facade01")},
    {TEXT("building_modern"), TEXT("/Game/Carla/Static/GenericMaterials/Facade/MI_Facade01.MI_Facade01")},
    {TEXT("building_plaster"), TEXT("/Game/Carla/Static/GenericMaterials/Facade/MI_Facade01.MI_Facade01")},
    {TEXT("building_stone"), TEXT("/Game/Carla/Static/Building/Buildings/BuildingComplete/00_IndividualComponents/Apt34/MI_Apt34.MI_Apt34")},
    {TEXT("roof"), TEXT("/Game/Carla/Static/Building/Buildings/BuildingComplete/00_GenericComponents/Apt32_to_35/MI_Apt_Roof.MI_Apt_Roof")},
    {TEXT("roof_tiles"), TEXT("/Game/Carla/Static/Building/Buildings/BuildingComplete/00_GenericComponents/Apt32_to_35/MI_Apt_Roof.MI_Apt_Roof")},
    {TEXT("roof_bitumen"), TEXT("/Game/Carla/Static/GenericMaterials/00_LegacyMaterials/Building/RoofBitumen/MI_RoofBitumen_02.MI_RoofBitumen_02")},
    {TEXT("roof_metal"), TEXT("/Game/Carla/Static/GenericMaterials/00_LegacyMaterials/Building/RoofBitumen/MI_RoofBitumen_02.MI_RoofBitumen_02")},

    // Ground & nature
    {TEXT("terrain"), TEXT("/Game/Carla/Static/GenericMaterials/Ground/MI_LargeLandscape_Grass.MI_LargeLandscape_Grass")},
    {TEXT("grass"), TEXT("/Game/Carla/Static/GenericMaterials/Ground/MI_LargeLandscape_Grass.MI_LargeLandscape_Grass")},
    {TEXT("dirt"), TEXT("/Game/Carla/Static/GenericMaterials/Ground/MI_Dirt.MI_Dirt")},
    {TEXT("sand"), TEXT("/Game/Carla/Static/GenericMaterials/Ground/MI_Dirt.MI_Dirt")},
    {TEXT("gravel"), TEXT("/Game/Carla/Static/GenericMaterials/Ground/MI_LargeLandscape_Interurban.MI_LargeLandscape_Interurban")},
    {TEXT("rock"), TEXT("/Game/Carla/Static/GenericMaterials/Ground/MI_LargeLandscape_Interurban.MI_LargeLandscape_Interurban")},
    {TEXT("water"), TEXT("/Game/Carla/Static/GenericMaterials/Water/MI_WaterLake01.MI_WaterLake01")},

    // Walls, fences & metal
    {TEXT("wall_concrete"), TEXT("/Game/Carla/Static/Wall/00_LegacyAssets/Materials/Wall04/MI_Wall04.MI_Wall04")},
    {TEXT("wall_brick"), TEXT("/Game/Carla/Static/Building/Buildings/BuildingComplete/00_GenericComponents/Apt32_to_35/MI_Apt_Bricks.MI_Apt_Bricks")},
    {TEXT("wall_stone"), TEXT("/Game/Carla/Static/GenericMaterials/Wall/MI_CityWall01.MI_CityWall01")},
    {TEXT("fence"), TEXT("/Game/Carla/Static/GenericMaterials/Metal/MI_Metal_Base.MI_Metal_Base")},
    {TEXT("fence_metal"), TEXT("/Game/Carla/Static/GenericMaterials/Metal/MI_Metal_Base.MI_Metal_Base")},
    {TEXT("fence_wood"), TEXT("/Game/Carla/Static/GenericMaterials/00_LegacyMaterials/Wood/MI_Wood01.MI_Wood01")},
    {TEXT("hedge"), TEXT("/Game/Carla/Static/Vegetation/00_LegacyAssets/Materials/Bush01/M_Bush01.M_Bush01")},
    {TEXT("guardrail"), TEXT("/Game/Carla/Static/Static/00_LegacyAssets/Materials/ParkingBarrier/MI_ParkingBarrier_Opt.MI_ParkingBarrier_Opt")},

    // Road markings & lane lines
    {TEXT("marking_white"), TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Asphalt_B_LaneMarkingWhite.MI_Road_Asphalt_B_LaneMarkingWhite")},
    {TEXT("marking_yellow"), TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Asphalt_B_LaneMarkingYellow.MI_Road_Asphalt_B_LaneMarkingYellow")},

    // Custom PBR
    {TEXT("custom_pbr"), TEXT("/Game/Carla/Static/GenericMaterials/00_LegacyMaterials/Concrete/MI_Concrete_01.MI_Concrete_01")},
};

carla::rpc::CityObjectLabel FCarlaMapBundle::ParseSemanticLabel(const FString &LabelStr)
{
  const FString Trimmed = LabelStr.TrimStartAndEnd();
  if (Trimmed.Equals(TEXT("Roads"), ESearchCase::IgnoreCase) || Trimmed.Equals(TEXT("Road"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::Roads;
  if (Trimmed.Equals(TEXT("Sidewalks"), ESearchCase::IgnoreCase) || Trimmed.Equals(TEXT("Sidewalk"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::Sidewalks;
  if (Trimmed.Equals(TEXT("Buildings"), ESearchCase::IgnoreCase) || Trimmed.Equals(TEXT("Building"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::Buildings;
  if (Trimmed.Equals(TEXT("Walls"), ESearchCase::IgnoreCase) || Trimmed.Equals(TEXT("Wall"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::Walls;
  if (Trimmed.Equals(TEXT("Fences"), ESearchCase::IgnoreCase) || Trimmed.Equals(TEXT("Fence"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::Fences;
  if (Trimmed.Equals(TEXT("Poles"), ESearchCase::IgnoreCase) || Trimmed.Equals(TEXT("Pole"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::Poles;
  if (Trimmed.Equals(TEXT("TrafficLight"), ESearchCase::IgnoreCase) || Trimmed.Equals(TEXT("TrafficLights"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::TrafficLight;
  if (Trimmed.Equals(TEXT("TrafficSigns"), ESearchCase::IgnoreCase) || Trimmed.Equals(TEXT("TrafficSign"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::TrafficSigns;
  if (Trimmed.Equals(TEXT("Vegetation"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::Vegetation;
  if (Trimmed.Equals(TEXT("Terrain"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::Terrain;
  if (Trimmed.Equals(TEXT("Ground"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::Ground;
  if (Trimmed.Equals(TEXT("Water"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::Water;
  if (Trimmed.Equals(TEXT("RoadLines"), ESearchCase::IgnoreCase) || Trimmed.Equals(TEXT("RoadLine"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::RoadLines;
  if (Trimmed.Equals(TEXT("Bridge"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::Bridge;
  if (Trimmed.Equals(TEXT("RailTrack"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::RailTrack;
  if (Trimmed.Equals(TEXT("GuardRail"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::GuardRail;
  if (Trimmed.Equals(TEXT("Rock"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::Rock;
  if (Trimmed.Equals(TEXT("Static"), ESearchCase::IgnoreCase))
    return carla::rpc::CityObjectLabel::Static;

  // Numeric fallback
  if (Trimmed.IsNumeric())
  {
    int32 Val = FCString::Atoi(*Trimmed);
    return static_cast<carla::rpc::CityObjectLabel>(FMath::Clamp(Val, 0, 255));
  }

  return carla::rpc::CityObjectLabel::None;
}

UMaterialInterface *FCarlaMapBundle::ResolveMaterial(
    UObject *Outer,
    const FString &Preset,
    const carla::rpc::CustomMaterialParameters &Params)
{
  const FString PresetName = Preset.TrimStartAndEnd();
  const FString LowerPreset = PresetName.ToLower();
  const FString *FoundPath = GCarlaBundleMaterialCatalog.Find(LowerPreset);
  const FString ResolvedPath = (FoundPath != nullptr)
      ? *FoundPath
      : (PresetName.IsEmpty()
          ? GCarlaBundleMaterialCatalog[TEXT("building_concrete")]
          : PresetName);

  UMaterialInterface *BaseMat = LoadObject<UMaterialInterface>(nullptr, *ResolvedPath);
  if (BaseMat == nullptr)
  {
    BaseMat = LoadObject<UMaterialInterface>(
        nullptr, *GCarlaBundleMaterialCatalog[TEXT("building_concrete")]);
  }
  if (BaseMat == nullptr)
  {
    return nullptr;
  }

  const bool bHasCustomTextures =
      !Params.albedo_texture_path.empty() ||
      !Params.normal_texture_path.empty() ||
      !Params.roughness_texture_path.empty();
  const bool bHasCustomScalars =
      LowerPreset == TEXT("custom_pbr") ||
      Params.base_color.r != 200 || Params.base_color.g != 200 || Params.base_color.b != 200 ||
      FMath::Abs(Params.roughness - 0.8f) > 1e-4f ||
      FMath::Abs(Params.metallic - 0.0f) > 1e-4f ||
      FMath::Abs(Params.specular - 0.5f) > 1e-4f;

  if (!bHasCustomTextures && !bHasCustomScalars)
  {
    return BaseMat;
  }

  UMaterialInstanceDynamic *DynMat = UMaterialInstanceDynamic::Create(BaseMat, Outer);
  if (DynMat == nullptr)
  {
    return BaseMat;
  }

  const FLinearColor LinearColor = FLinearColor(FColor(
      Params.base_color.r, Params.base_color.g, Params.base_color.b, 255));
  for (const TCHAR *Param : {TEXT("BaseColor"), TEXT("Color"), TEXT("Tint"), TEXT("AlbedoTint"), TEXT("Base Color")})
  {
    DynMat->SetVectorParameterValue(FName(Param), LinearColor);
  }
  for (const TCHAR *Param : {TEXT("Roughness"), TEXT("RoughnessScale"), TEXT("RoughnessMult")})
  {
    DynMat->SetScalarParameterValue(FName(Param), Params.roughness);
  }
  for (const TCHAR *Param : {TEXT("Metallic"), TEXT("Metalness")})
  {
    DynMat->SetScalarParameterValue(FName(Param), Params.metallic);
  }
  for (const TCHAR *Param : {TEXT("Specular")})
  {
    DynMat->SetScalarParameterValue(FName(Param), Params.specular);
  }

  auto ApplyTextureIfValid = [&](const std::string &PathStr, std::initializer_list<const TCHAR *> ParamNames)
  {
    if (PathStr.empty())
    {
      return;
    }
    const FString FilePath = carla::rpc::ToFString(PathStr);
    if (!FPaths::FileExists(FilePath))
    {
      return;
    }
    if (UTexture2D *Tex = FImageUtils::ImportFileAsTexture2D(FilePath))
    {
      for (const TCHAR *Param : ParamNames)
      {
        DynMat->SetTextureParameterValue(FName(Param), Tex);
      }
    }
  };

  ApplyTextureIfValid(Params.albedo_texture_path, {TEXT("BaseColor"), TEXT("Albedo"), TEXT("Diffuse"), TEXT("BaseColorMap")});
  ApplyTextureIfValid(Params.normal_texture_path, {TEXT("Normal"), TEXT("NormalMap"), TEXT("Normals")});
  ApplyTextureIfValid(Params.roughness_texture_path, {TEXT("Roughness"), TEXT("RoughnessMap"), TEXT("ORM")});

  return DynMat;
}

bool FCarlaMapBundle::LoadCMeshFromMemory(
    const uint8 *Data,
    int64 DataLen,
    TArray<FVector> &OutVertices,
    TArray<int32> &OutTriangles,
    TArray<FVector> &OutNormals,
    TArray<FVector2D> &OutUV0,
    TArray<FLinearColor> &OutColors,
    FString &OutError)
{
  if (Data == nullptr || DataLen < static_cast<int64>(sizeof(FCMeshHeader)))
  {
    OutError = FString::Printf(TEXT("CMesh: data too short (%lld bytes, expected >= %zu)"),
        DataLen, sizeof(FCMeshHeader));
    return false;
  }

  const FCMeshHeader *Header = reinterpret_cast<const FCMeshHeader*>(Data);
  if (Header->Magic != CMESH_MAGIC)
  {
    OutError = FString::Printf(TEXT("CMesh: invalid magic 0x%08X (expected 0x%08X 'CMES')"),
        Header->Magic, CMESH_MAGIC);
    return false;
  }

  if (Header->Version != CMESH_VERSION_1)
  {
    OutError = FString::Printf(TEXT("CMesh: unsupported version %u (supported: 1)"), Header->Version);
    return false;
  }

  if (Header->VertexCount == 0)
  {
    OutError = TEXT("CMesh: vertex count is zero");
    return false;
  }

  if (Header->IndexCount == 0 || (Header->IndexCount % 3) != 0)
  {
    OutError = FString::Printf(TEXT("CMesh: invalid index count %u (must be multiple of 3)"),
        Header->IndexCount);
    return false;
  }

  const bool bHasNormals = (Header->Flags & CMESH_FLAG_HAS_NORMALS) != 0;
  const bool bHasUVs     = (Header->Flags & CMESH_FLAG_HAS_UVS) != 0;
  const bool bHasColors  = (Header->Flags & CMESH_FLAG_HAS_COLORS) != 0;

  int64 ExpectedSize = sizeof(FCMeshHeader);
  ExpectedSize += static_cast<int64>(Header->VertexCount) * 3 * sizeof(float);
  if (bHasNormals) ExpectedSize += static_cast<int64>(Header->VertexCount) * 3 * sizeof(float);
  if (bHasUVs)     ExpectedSize += static_cast<int64>(Header->VertexCount) * 2 * sizeof(float);
  if (bHasColors)  ExpectedSize += static_cast<int64>(Header->VertexCount) * 4 * sizeof(uint8);
  ExpectedSize += static_cast<int64>(Header->IndexCount) * sizeof(uint32);

  if (DataLen < ExpectedSize)
  {
    OutError = FString::Printf(TEXT("CMesh: file truncated (expected %lld bytes, got %lld)"),
        ExpectedSize, DataLen);
    return false;
  }

  const uint8 *Ptr = Data + sizeof(FCMeshHeader);

  // Vertices (metres in CARLA left-handed frame -> Unreal cm)
  const float *VertData = reinterpret_cast<const float*>(Ptr);
  OutVertices.Empty(Header->VertexCount);
  for (uint32 i = 0; i < Header->VertexCount; ++i)
  {
    OutVertices.Add(FVector(
        VertData[3 * i + 0] * 100.0f,
        VertData[3 * i + 1] * 100.0f,
        VertData[3 * i + 2] * 100.0f));
  }
  Ptr += static_cast<int64>(Header->VertexCount) * 3 * sizeof(float);

  // Normals
  OutNormals.Empty(Header->VertexCount);
  if (bHasNormals)
  {
    const float *NormData = reinterpret_cast<const float*>(Ptr);
    for (uint32 i = 0; i < Header->VertexCount; ++i)
    {
      OutNormals.Add(FVector(
          NormData[3 * i + 0],
          NormData[3 * i + 1],
          NormData[3 * i + 2]).GetSafeNormal(1e-8, FVector::UpVector));
    }
    Ptr += static_cast<int64>(Header->VertexCount) * 3 * sizeof(float);
  }

  // UVs
  OutUV0.Empty(Header->VertexCount);
  if (bHasUVs)
  {
    const float *UVData = reinterpret_cast<const float*>(Ptr);
    for (uint32 i = 0; i < Header->VertexCount; ++i)
    {
      OutUV0.Add(FVector2D(UVData[2 * i + 0], UVData[2 * i + 1]));
    }
    Ptr += static_cast<int64>(Header->VertexCount) * 2 * sizeof(float);
  }

  // Colors
  OutColors.Empty(Header->VertexCount);
  if (bHasColors)
  {
    const uint8 *ColorData = Ptr;
    for (uint32 i = 0; i < Header->VertexCount; ++i)
    {
      OutColors.Add(FLinearColor(FColor(
          ColorData[4 * i + 0],
          ColorData[4 * i + 1],
          ColorData[4 * i + 2],
          ColorData[4 * i + 3])));
    }
    Ptr += static_cast<int64>(Header->VertexCount) * 4 * sizeof(uint8);
  }

  // Indices
  const uint32 *IndexData = reinterpret_cast<const uint32*>(Ptr);
  OutTriangles.Empty(Header->IndexCount);
  for (uint32 i = 0; i < Header->IndexCount; ++i)
  {
    const uint32 Idx = IndexData[i];
    if (Idx >= Header->VertexCount)
    {
      OutError = FString::Printf(TEXT("CMesh: triangle index out of range (%u >= %u)"),
          Idx, Header->VertexCount);
      return false;
    }
    OutTriangles.Add(static_cast<int32>(Idx));
  }

  // Fallback normals if missing
  if (!bHasNormals)
  {
    OutNormals.Init(FVector::ZeroVector, OutVertices.Num());
    for (int32 i = 0; i + 2 < OutTriangles.Num(); i += 3)
    {
      const FVector &V0 = OutVertices[OutTriangles[i]];
      const FVector &V1 = OutVertices[OutTriangles[i + 1]];
      const FVector &V2 = OutVertices[OutTriangles[i + 2]];
      const FVector FaceNormal = FVector::CrossProduct(V2 - V0, V1 - V0);
      OutNormals[OutTriangles[i]] += FaceNormal;
      OutNormals[OutTriangles[i + 1]] += FaceNormal;
      OutNormals[OutTriangles[i + 2]] += FaceNormal;
    }
    for (FVector &N : OutNormals)
    {
      N = N.GetSafeNormal(1e-8, FVector::UpVector);
    }
  }

  // Fallback planar UVs if missing
  if (!bHasUVs)
  {
    for (int32 i = 0; i < OutVertices.Num(); ++i)
    {
      const FVector &V = OutVertices[i];
      const FVector &N = OutNormals[i];
      if (FMath::Abs(N.Z) >= 0.5f)
      {
        OutUV0.Add(FVector2D(V.X / 500.0f, V.Y / 500.0f));
      }
      else if (FMath::Abs(N.X) >= FMath::Abs(N.Y))
      {
        OutUV0.Add(FVector2D(V.Y / 500.0f, V.Z / 500.0f));
      }
      else
      {
        OutUV0.Add(FVector2D(V.X / 500.0f, V.Z / 500.0f));
      }
    }
  }

  return true;
}

bool FCarlaMapBundle::LoadCMesh(
    const FString &FilePath,
    TArray<FVector> &OutVertices,
    TArray<int32> &OutTriangles,
    TArray<FVector> &OutNormals,
    TArray<FVector2D> &OutUV0,
    TArray<FLinearColor> &OutColors,
    FString &OutError)
{
  TArray<uint8> FileData;
  if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
  {
    OutError = FString::Printf(TEXT("CMesh: unable to open file '%s'"), *FilePath);
    return false;
  }
  return LoadCMeshFromMemory(
      FileData.GetData(),
      FileData.Num(),
      OutVertices,
      OutTriangles,
      OutNormals,
      OutUV0,
      OutColors,
      OutError);
}

TMap<FString, FString> FCarlaMapBundle::FindAllBundles()
{
  TMap<FString, FString> Discovered;
  IFileManager &FileManager = IFileManager::Get();

  TArray<FString> SearchDirs;
  SearchDirs.Add(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Maps")));
  SearchDirs.Add(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Maps")));

  FString CustomMapsDir;
  if (FParse::Value(FCommandLine::Get(), TEXT("-carla-maps="), CustomMapsDir))
  {
    SearchDirs.Add(FPaths::ConvertRelativePathToFull(CustomMapsDir));
  }

  FString EnvMapsPath = FPlatformMisc::GetEnvironmentVariable(TEXT("CARLA_MAPS_PATH"));
  if (!EnvMapsPath.IsEmpty())
  {
    TArray<FString> EnvDirs;
    EnvMapsPath.ParseIntoArray(EnvDirs, TEXT(":"), true);
    for (const FString &EDir : EnvDirs)
    {
      SearchDirs.Add(FPaths::ConvertRelativePathToFull(EDir));
    }
  }

  for (const FString &SearchDir : SearchDirs)
  {
    if (!FileManager.DirectoryExists(*SearchDir))
    {
      continue;
    }

    TArray<FString> SubDirs;
    FileManager.FindFiles(SubDirs, *(SearchDir / TEXT("*")), false, true);

    for (const FString &SubDirName : SubDirs)
    {
      const FString FullSubDirPath = SearchDir / SubDirName;
      const FString JsonPath = FullSubDirPath / TEXT("carla-map.json");
      if (FileManager.FileExists(*JsonPath))
      {
        FString MapName = SubDirName;
        if (MapName.EndsWith(TEXT(".carlamap")))
        {
          MapName = MapName.LeftChop(9);
        }
        Discovered.Add(MapName, FullSubDirPath);
      }
    }

    const FString RootJson = SearchDir / TEXT("carla-map.json");
    if (FileManager.FileExists(*RootJson))
    {
      FString MapName = FPaths::GetCleanFilename(SearchDir);
      if (MapName.EndsWith(TEXT(".carlamap")))
      {
        MapName = MapName.LeftChop(9);
      }
      Discovered.Add(MapName, SearchDir);
    }
  }

  return Discovered;
}

bool FCarlaMapBundle::LoadBundleFromDir(
    const FString &Dir,
    FCarlaMapBundle &OutBundle,
    FString &OutError)
{
  const FString JsonPath = Dir / TEXT("carla-map.json");
  if (!FPaths::FileExists(JsonPath))
  {
    OutError = FString::Printf(TEXT("carla-map.json not found in '%s'"), *Dir);
    return false;
  }

  FString JsonStr;
  if (!FFileHelper::LoadFileToString(JsonStr, *JsonPath))
  {
    OutError = FString::Printf(TEXT("failed to read '%s'"), *JsonPath);
    return false;
  }

  TSharedPtr<FJsonObject> JsonObj;
  TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
  if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
  {
    OutError = FString::Printf(TEXT("invalid JSON syntax in '%s'"), *JsonPath);
    return false;
  }

  OutBundle.BundleDir = FPaths::ConvertRelativePathToFull(Dir);

  if (JsonObj->HasTypedField<EJson::String>(TEXT("map_name")))
  {
    OutBundle.MapName = JsonObj->GetStringField(TEXT("map_name"));
  }
  else
  {
    OutBundle.MapName = FPaths::GetCleanFilename(Dir);
    if (OutBundle.MapName.EndsWith(TEXT(".carlamap")))
    {
      OutBundle.MapName = OutBundle.MapName.LeftChop(9);
    }
  }

  if (JsonObj->HasTypedField<EJson::String>(TEXT("xodr_file")))
  {
    OutBundle.XodrFile = JsonObj->GetStringField(TEXT("xodr_file"));
  }
  else
  {
    OutBundle.XodrFile = FString::Printf(TEXT("OpenDrive/%s.xodr"), *OutBundle.MapName);
  }

  if (JsonObj->HasTypedField<EJson::String>(TEXT("nav_file")))
  {
    OutBundle.NavFile = JsonObj->GetStringField(TEXT("nav_file"));
  }
  else
  {
    const FString DefaultNav = Dir / FString::Printf(TEXT("Nav/%s.bin"), *OutBundle.MapName);
    if (FPaths::FileExists(DefaultNav))
    {
      OutBundle.NavFile = FString::Printf(TEXT("Nav/%s.bin"), *OutBundle.MapName);
    }
  }

  // OpenDrive parameters
  OutBundle.GenerationParams = carla::rpc::OpendriveGenerationParameters{};
  OutBundle.GenerationParams.generate_road_mesh = false; // default false for custom mesh bundles
  OutBundle.GenerationParams.generate_ground_mesh = false;
  OutBundle.GenerationParams.generate_traffic_signals = true;
  OutBundle.GenerationParams.generate_street_furniture = false;
  OutBundle.GenerationParams.wall_height = 0.0;

  const TSharedPtr<FJsonObject> *ParamsObj;
  if (JsonObj->TryGetObjectField(TEXT("opendrive_parameters"), ParamsObj))
  {
    if ((*ParamsObj)->HasField(TEXT("generate_road_mesh")))
      OutBundle.GenerationParams.generate_road_mesh = (*ParamsObj)->GetBoolField(TEXT("generate_road_mesh"));
    if ((*ParamsObj)->HasField(TEXT("generate_ground_mesh")))
      OutBundle.GenerationParams.generate_ground_mesh = (*ParamsObj)->GetBoolField(TEXT("generate_ground_mesh"));
    if ((*ParamsObj)->HasField(TEXT("generate_traffic_signals")))
      OutBundle.GenerationParams.generate_traffic_signals = (*ParamsObj)->GetBoolField(TEXT("generate_traffic_signals"));
    if ((*ParamsObj)->HasField(TEXT("generate_street_furniture")))
      OutBundle.GenerationParams.generate_street_furniture = (*ParamsObj)->GetBoolField(TEXT("generate_street_furniture"));
    if ((*ParamsObj)->HasField(TEXT("vertex_distance")))
      OutBundle.GenerationParams.vertex_distance = static_cast<float>((*ParamsObj)->GetNumberField(TEXT("vertex_distance")));
    if ((*ParamsObj)->HasField(TEXT("max_road_length")))
      OutBundle.GenerationParams.max_road_length = static_cast<float>((*ParamsObj)->GetNumberField(TEXT("max_road_length")));
    if ((*ParamsObj)->HasField(TEXT("wall_height")))
      OutBundle.GenerationParams.wall_height = static_cast<float>((*ParamsObj)->GetNumberField(TEXT("wall_height")));
  }

  // Custom mesh layers
  OutBundle.Layers.Empty();
  const TArray<TSharedPtr<FJsonValue>> *LayersArray;
  if (JsonObj->TryGetArrayField(TEXT("layers"), LayersArray))
  {
    for (const auto &Val : *LayersArray)
    {
      const TSharedPtr<FJsonObject> LayerObj = Val->AsObject();
      if (!LayerObj.IsValid()) continue;

      FCarlaMapLayer Layer;
      Layer.Name = LayerObj->GetStringField(TEXT("name"));
      Layer.MeshFile = LayerObj->GetStringField(TEXT("mesh_file"));

      if (LayerObj->HasField(TEXT("semantic_label")))
      {
        Layer.SemanticLabel = ParseSemanticLabel(LayerObj->GetStringField(TEXT("semantic_label")));
      }
      else
      {
        Layer.SemanticLabel = ParseSemanticLabel(Layer.Name);
      }

      if (LayerObj->HasField(TEXT("material_preset")))
      {
        Layer.MaterialPreset = LayerObj->GetStringField(TEXT("material_preset"));
      }

      if (LayerObj->HasField(TEXT("enable_collision")))
      {
        Layer.bEnableCollision = LayerObj->GetBoolField(TEXT("enable_collision"));
      }

      const TSharedPtr<FJsonObject> *TransformObj;
      if (LayerObj->TryGetObjectField(TEXT("transform"), TransformObj))
      {
        FVector Loc = FVector::ZeroVector;
        FRotator Rot = FRotator::ZeroRotator;
        FVector Scale = FVector::OneVector;

        const TArray<TSharedPtr<FJsonValue>> *LocArr;
        if ((*TransformObj)->TryGetArrayField(TEXT("location"), LocArr) && LocArr->Num() >= 3)
        {
          Loc = FVector((*LocArr)[0]->AsNumber() * 100.0f, (*LocArr)[1]->AsNumber() * 100.0f, (*LocArr)[2]->AsNumber() * 100.0f);
        }
        const TArray<TSharedPtr<FJsonValue>> *RotArr;
        if ((*TransformObj)->TryGetArrayField(TEXT("rotation"), RotArr) && RotArr->Num() >= 3)
        {
          Rot = FRotator((*RotArr)[0]->AsNumber(), (*RotArr)[1]->AsNumber(), (*RotArr)[2]->AsNumber());
        }
        const TArray<TSharedPtr<FJsonValue>> *ScaleArr;
        if ((*TransformObj)->TryGetArrayField(TEXT("scale"), ScaleArr) && ScaleArr->Num() >= 3)
        {
          Scale = FVector((*ScaleArr)[0]->AsNumber(), (*ScaleArr)[1]->AsNumber(), (*ScaleArr)[2]->AsNumber());
        }
        Layer.Transform = FTransform(Rot, Loc, Scale);
      }

      const TSharedPtr<FJsonObject> *MatParamsObj;
      if (LayerObj->TryGetObjectField(TEXT("material_params"), MatParamsObj))
      {
        if ((*MatParamsObj)->HasField(TEXT("roughness")))
          Layer.MaterialParams.roughness = static_cast<float>((*MatParamsObj)->GetNumberField(TEXT("roughness")));
        if ((*MatParamsObj)->HasField(TEXT("metallic")))
          Layer.MaterialParams.metallic = static_cast<float>((*MatParamsObj)->GetNumberField(TEXT("metallic")));
        if ((*MatParamsObj)->HasField(TEXT("specular")))
          Layer.MaterialParams.specular = static_cast<float>((*MatParamsObj)->GetNumberField(TEXT("specular")));
        if ((*MatParamsObj)->HasField(TEXT("albedo_texture_path")))
          Layer.MaterialParams.albedo_texture_path = carla::rpc::FromFString((*MatParamsObj)->GetStringField(TEXT("albedo_texture_path")));
        if ((*MatParamsObj)->HasField(TEXT("normal_texture_path")))
          Layer.MaterialParams.normal_texture_path = carla::rpc::FromFString((*MatParamsObj)->GetStringField(TEXT("normal_texture_path")));
        if ((*MatParamsObj)->HasField(TEXT("roughness_texture_path")))
          Layer.MaterialParams.roughness_texture_path = carla::rpc::FromFString((*MatParamsObj)->GetStringField(TEXT("roughness_texture_path")));

        const TArray<TSharedPtr<FJsonValue>> *UVScaleArr;
        if ((*MatParamsObj)->TryGetArrayField(TEXT("uv_scale"), UVScaleArr) && UVScaleArr->Num() >= 2)
        {
          Layer.MaterialParams.uv_scale = {
              static_cast<float>((*UVScaleArr)[0]->AsNumber()),
              static_cast<float>((*UVScaleArr)[1]->AsNumber())};
        }
        const TArray<TSharedPtr<FJsonValue>> *ColorArr;
        if ((*MatParamsObj)->TryGetArrayField(TEXT("base_color"), ColorArr) && ColorArr->Num() >= 3)
        {
          Layer.MaterialParams.base_color = {
              static_cast<uint8_t>(FMath::Clamp(static_cast<int32>((*ColorArr)[0]->AsNumber()), 0, 255)),
              static_cast<uint8_t>(FMath::Clamp(static_cast<int32>((*ColorArr)[1]->AsNumber()), 0, 255)),
              static_cast<uint8_t>(FMath::Clamp(static_cast<int32>((*ColorArr)[2]->AsNumber()), 0, 255))};
        }
      }

      OutBundle.Layers.Add(MoveTemp(Layer));
    }
  }

  // Placements
  OutBundle.Placements.Empty();
  auto ParsePlacements = [&](const TArray<TSharedPtr<FJsonValue>> &Arr)
  {
    for (const auto &Val : Arr)
    {
      const TSharedPtr<FJsonObject> PlaceObj = Val->AsObject();
      if (!PlaceObj.IsValid()) continue;

      FCarlaMapPlacement Placement;
      Placement.AssetPath = PlaceObj->GetStringField(TEXT("asset_path"));

      const TArray<TSharedPtr<FJsonValue>> *TransArr;
      if (PlaceObj->TryGetArrayField(TEXT("transforms"), TransArr))
      {
        for (const auto &TVal : *TransArr)
        {
          const TSharedPtr<FJsonObject> TObj = TVal->AsObject();
          if (!TObj.IsValid()) continue;

          FVector Loc = FVector::ZeroVector;
          FRotator Rot = FRotator::ZeroRotator;
          FVector Scale = FVector::OneVector;

          const TArray<TSharedPtr<FJsonValue>> *LocArr;
          if (TObj->TryGetArrayField(TEXT("location"), LocArr) && LocArr->Num() >= 3)
          {
            Loc = FVector((*LocArr)[0]->AsNumber() * 100.0f, (*LocArr)[1]->AsNumber() * 100.0f, (*LocArr)[2]->AsNumber() * 100.0f);
          }
          const TArray<TSharedPtr<FJsonValue>> *RotArr;
          if (TObj->TryGetArrayField(TEXT("rotation"), RotArr) && RotArr->Num() >= 3)
          {
            Rot = FRotator((*RotArr)[0]->AsNumber(), (*RotArr)[1]->AsNumber(), (*RotArr)[2]->AsNumber());
          }
          const TArray<TSharedPtr<FJsonValue>> *ScaleArr;
          if (TObj->TryGetArrayField(TEXT("scale"), ScaleArr) && ScaleArr->Num() >= 3)
          {
            Scale = FVector((*ScaleArr)[0]->AsNumber(), (*ScaleArr)[1]->AsNumber(), (*ScaleArr)[2]->AsNumber());
          }
          Placement.Transforms.Add(FTransform(Rot, Loc, Scale));
        }
      }
      OutBundle.Placements.Add(MoveTemp(Placement));
    }
  };

  const TArray<TSharedPtr<FJsonValue>> *PlacementsArray;
  if (JsonObj->TryGetArrayField(TEXT("placements"), PlacementsArray))
  {
    ParsePlacements(*PlacementsArray);
  }

  // External placements file
  if (JsonObj->HasTypedField<EJson::String>(TEXT("placements_file")))
  {
    const FString ExtPlacementsPath = Dir / JsonObj->GetStringField(TEXT("placements_file"));
    FString ExtPlacementsStr;
    if (FFileHelper::LoadFileToString(ExtPlacementsStr, *ExtPlacementsPath))
    {
      TSharedPtr<FJsonObject> ExtObj;
      TSharedRef<TJsonReader<>> ExtReader = TJsonReaderFactory<>::Create(ExtPlacementsStr);
      if (FJsonSerializer::Deserialize(ExtReader, ExtObj) && ExtObj.IsValid())
      {
        const TArray<TSharedPtr<FJsonValue>> *ExtArr;
        if (ExtObj->TryGetArrayField(TEXT("placements"), ExtArr))
        {
          ParsePlacements(*ExtArr);
        }
      }
    }
  }

  // Custom spawn points
  OutBundle.CustomSpawnPoints.Empty();
  const TArray<TSharedPtr<FJsonValue>> *SpawnArr;
  if (JsonObj->TryGetArrayField(TEXT("spawn_points"), SpawnArr))
  {
    for (const auto &SVal : *SpawnArr)
    {
      const TSharedPtr<FJsonObject> SObj = SVal->AsObject();
      if (!SObj.IsValid()) continue;

      FVector Loc = FVector::ZeroVector;
      FRotator Rot = FRotator::ZeroRotator;
      const TArray<TSharedPtr<FJsonValue>> *LocArr;
      if (SObj->TryGetArrayField(TEXT("location"), LocArr) && LocArr->Num() >= 3)
      {
        Loc = FVector((*LocArr)[0]->AsNumber() * 100.0f, (*LocArr)[1]->AsNumber() * 100.0f, (*LocArr)[2]->AsNumber() * 100.0f);
      }
      const TArray<TSharedPtr<FJsonValue>> *RotArr;
      if (SObj->TryGetArrayField(TEXT("rotation"), RotArr) && RotArr->Num() >= 3)
      {
        Rot = FRotator((*RotArr)[0]->AsNumber(), (*RotArr)[1]->AsNumber(), (*RotArr)[2]->AsNumber());
      }
      OutBundle.CustomSpawnPoints.Add(FTransform(Rot, Loc));
    }
  }

  return true;
}

bool FCarlaMapBundle::StageBundle(FString &OutError) const
{
  IFileManager &FileManager = IFileManager::Get();

  // 1. Stage OpenDRIVE (.xodr)
  FString SrcXodr = BundleDir / XodrFile;
  if (!FileManager.FileExists(*SrcXodr))
  {
    SrcXodr = BundleDir / FString::Printf(TEXT("OpenDrive/%s.xodr"), *MapName);
  }
  if (!FileManager.FileExists(*SrcXodr))
  {
    OutError = FString::Printf(TEXT("OpenDRIVE file '%s' not found in bundle '%s'"), *XodrFile, *BundleDir);
    return false;
  }

  FString XodrContent;
  if (!FFileHelper::LoadFileToString(XodrContent, *SrcXodr))
  {
    OutError = FString::Printf(TEXT("Failed to read OpenDRIVE file '%s'"), *SrcXodr);
    return false;
  }

  const FString SavedOpenDriveDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("OpenDrive"));
  FileManager.MakeDirectory(*SavedOpenDriveDir, true);

  // Write to both OpenDriveMap.xodr (for AOpenDriveGenerator) and <MapName>.xodr
  const FString DstOpenDriveMapXodr = SavedOpenDriveDir / TEXT("OpenDriveMap.xodr");
  const FString DstNamedXodr = SavedOpenDriveDir / (MapName + TEXT(".xodr"));

  FFileHelper::SaveStringToFile(XodrContent, *DstOpenDriveMapXodr, FFileHelper::EEncodingOptions::ForceUTF8, &FileManager);
  FFileHelper::SaveStringToFile(XodrContent, *DstNamedXodr, FFileHelper::EEncodingOptions::ForceUTF8, &FileManager);

  // 2. Stage pre-compiled NavMesh (.bin) if available
  FString SrcNav = BundleDir / NavFile;
  if (NavFile.IsEmpty() || !FileManager.FileExists(*SrcNav))
  {
    SrcNav = BundleDir / FString::Printf(TEXT("Nav/%s.bin"), *MapName);
  }

  if (FileManager.FileExists(*SrcNav))
  {
    TArray<uint8> NavData;
    if (FFileHelper::LoadFileToArray(NavData, *SrcNav))
    {
      const FString SavedNavDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Nav"));
      FileManager.MakeDirectory(*SavedNavDir, true);

      const FString DstOpenDriveMapNav = SavedNavDir / TEXT("OpenDriveMap.bin");
      const FString DstNamedNav = SavedNavDir / (MapName + TEXT(".bin"));

      FFileHelper::SaveArrayToFile(NavData, *DstOpenDriveMapNav);
      FFileHelper::SaveArrayToFile(NavData, *DstNamedNav);

      UE_LOG(LogCarla, Log, TEXT("FCarlaMapBundle: staged pre-compiled navmesh '%s' -> '%s' (%d bytes)"),
          *SrcNav, *DstNamedNav, NavData.Num());
    }
  }

  return true;
}

int32 FCarlaMapBundle::InstantiateInWorld(
    UWorld *World,
    ACarlaGameModeBase *GameMode,
    FString &OutError) const
{
  if (World == nullptr)
  {
    OutError = TEXT("InstantiateInWorld: World is null");
    return 0;
  }

  int32 SpawnedCount = 0;

  // 1. Instantiate .cmesh procedural mesh layers
  for (const FCarlaMapLayer &Layer : Layers)
  {
    const FString MeshPath = BundleDir / Layer.MeshFile;
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UV0;
    TArray<FLinearColor> Colors;
    FString LoadErr;

    if (!LoadCMesh(MeshPath, Vertices, Triangles, Normals, UV0, Colors, LoadErr))
    {
      UE_LOG(LogCarla, Error, TEXT("FCarlaMapBundle: layer '%s' failed to load cmesh '%s': %s"),
          *Layer.Name, *MeshPath, *LoadErr);
      continue;
    }

    // Scale vertices by Layer.Transform scale
    const FVector Scale = Layer.Transform.GetScale3D();
    for (FVector &V : Vertices)
    {
      V.X *= Scale.X;
      V.Y *= Scale.Y;
      V.Z *= Scale.Z;
    }

    // Apply UV scale if customized
    const float UVScaleX = FMath::Abs(Layer.MaterialParams.uv_scale.x) > 1e-4f ? Layer.MaterialParams.uv_scale.x : 1.0f;
    const float UVScaleY = FMath::Abs(Layer.MaterialParams.uv_scale.y) > 1e-4f ? Layer.MaterialParams.uv_scale.y : 1.0f;
    if (FMath::Abs(UVScaleX - 1.0f) > 1e-4f || FMath::Abs(UVScaleY - 1.0f) > 1e-4f)
    {
      for (FVector2D &UV : UV0)
      {
        UV.X *= UVScaleX;
        UV.Y *= UVScaleY;
      }
    }

    FTransform SpawnTransform = Layer.Transform;
    SpawnTransform.SetScale3D(FVector::OneVector); // Scale was baked into vertices

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ACarlaProceduralMeshActor *MeshActor = World->SpawnActor<ACarlaProceduralMeshActor>(
        ACarlaProceduralMeshActor::StaticClass(),
        SpawnTransform,
        SpawnParams);

    if (MeshActor == nullptr)
    {
      UE_LOG(LogCarla, Error, TEXT("FCarlaMapBundle: failed to spawn procedural mesh actor for layer '%s'"),
          *Layer.Name);
      continue;
    }

    MeshActor->CustomMeshName = Layer.Name;

    UProceduralMeshComponent *PMC = MeshActor->MeshComponent;
    PMC->bUseAsyncCooking = false;
    PMC->bUseComplexAsSimpleCollision = true;
    PMC->CreateMeshSection_LinearColor(
        0,
        Vertices,
        Triangles,
        Normals,
        UV0,
        Colors,
        TArray<FProcMeshTangent>(),
        Layer.bEnableCollision);

    if (Layer.bEnableCollision)
    {
      PMC->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
      PMC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
      PMC->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
      PMC->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
      PMC->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);
      PMC->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel3, ECollisionResponse::ECR_Overlap);
    }
    else
    {
      PMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (UMaterialInterface *Material = ResolveMaterial(MeshActor, Layer.MaterialPreset, Layer.MaterialParams))
    {
      PMC->SetMaterial(0, Material);
    }

    PMC->ComponentTags.Empty();
    ATagger::SetStencilValue(*PMC, MeshActor->GetUniqueID(), Layer.SemanticLabel, true);
    PMC->CustomDepthStencilValue = static_cast<int32>(Layer.SemanticLabel);
    PMC->ComponentTags.Add(FName(*ATagger::GetTagAsString(Layer.SemanticLabel)));
    PMC->UpdateBounds();
    PMC->MarkRenderStateDirty();

    if (GameMode != nullptr && GameMode->GetObjectRegister() != nullptr)
    {
      MeshActor->EnvironmentObjectId = GameMode->GetObjectRegister()->RegisterPMCComponents(MeshActor);
    }
    if (MeshActor->EnvironmentObjectId == 0)
    {
      const FString FallbackKey = MeshActor->GetName();
      MeshActor->EnvironmentObjectId = CityHash64(TCHAR_TO_ANSI(*FallbackKey), FallbackKey.Len());
    }

    SpawnedCount++;
  }

  // 2. Instantiate prop placements (HISM)
  for (const FCarlaMapPlacement &Placement : Placements)
  {
    if (Placement.AssetPath.IsEmpty() || Placement.Transforms.Num() == 0)
    {
      continue;
    }

    UStaticMesh *Mesh = LoadObject<UStaticMesh>(nullptr, *Placement.AssetPath);
    if (Mesh != nullptr)
    {
      FActorSpawnParameters SpawnParams;
      SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
      AActor *PropActor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
      if (PropActor != nullptr)
      {
        USceneComponent *RootComp = NewObject<USceneComponent>(PropActor, TEXT("RootComponent"));
        PropActor->SetRootComponent(RootComp);
        RootComp->RegisterComponent();

        UHierarchicalInstancedStaticMeshComponent *HISM =
            NewObject<UHierarchicalInstancedStaticMeshComponent>(PropActor, TEXT("HISMComponent"));
        HISM->SetupAttachment(RootComp);
        HISM->SetStaticMesh(Mesh);
        HISM->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
        HISM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        HISM->RegisterComponent();

        for (const FTransform &T : Placement.Transforms)
        {
          HISM->AddInstance(T);
        }
        HISM->UpdateBounds();
        HISM->MarkRenderStateDirty();

        carla::rpc::CityObjectLabel TagLabel = carla::rpc::CityObjectLabel::Static;
        if (Placement.AssetPath.Contains(TEXT("Building")) || Placement.AssetPath.Contains(TEXT("Apt")))
        {
          TagLabel = carla::rpc::CityObjectLabel::Buildings;
        }
        else if (Placement.AssetPath.Contains(TEXT("Tree")) || Placement.AssetPath.Contains(TEXT("Vegetation")))
        {
          TagLabel = carla::rpc::CityObjectLabel::Vegetation;
        }
        else if (Placement.AssetPath.Contains(TEXT("Vehicle")) || Placement.AssetPath.Contains(TEXT("Car")))
        {
          TagLabel = carla::rpc::CityObjectLabel::Car;
        }

        ATagger::SetStencilValue(*HISM, PropActor->GetUniqueID(), TagLabel, true);
        HISM->CustomDepthStencilValue = static_cast<int32>(TagLabel);
        HISM->ComponentTags.Add(FName(*ATagger::GetTagAsString(TagLabel)));
        PropActor->Tags.Add(FName(*ATagger::GetTagAsString(TagLabel)));

        SpawnedCount++;
      }
    }
  }

  // 3. Spawn custom vehicle spawn points
  for (const FTransform &SpawnTrans : CustomSpawnPoints)
  {
    AVehicleSpawnPoint *Spawner = World->SpawnActor<AVehicleSpawnPoint>();
    if (Spawner != nullptr)
    {
      Spawner->SetActorTransform(SpawnTrans);
      SpawnedCount++;
    }
  }

  return SpawnedCount;
}

