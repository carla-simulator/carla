// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TreeTableRow.h"
#include "SignDataAsset.generated.h"

/// Functional family of a traffic sign (mirrors the content enum
/// Enum_TrafficSignCategories and the atlas folder names).
UENUM(BlueprintType)
enum class ESignCategory : uint8
{
  Priority    UMETA(DisplayName = "Priority"),
  Mandatory   UMETA(DisplayName = "Mandatory"),
  Prohibitory UMETA(DisplayName = "Prohibitory"),
  Warning     UMETA(DisplayName = "Warning"),
  Information UMETA(DisplayName = "Information"),
  Guide       UMETA(DisplayName = "Guide"),
  Others      UMETA(DisplayName = "Others"),
  SpeedLimit  UMETA(DisplayName = "SpeedLimit"),
  None        UMETA(DisplayName = "None")
};

/**
 * One traffic sign of the catalog: a plate mesh plus the texture that paints it.
 *
 * The sign art lives in 4096^2 atlases of 4 x 4 cells sampled by
 * M_SignTextureAtlasSelector as uv = (TexCoord + (Id_X - 1, Id_Y - 1)) / 4, i.e. the
 * indices are 1-based, column then row, (1, 1) being the top-left cell. A "unique" sign
 * carries a standalone texture instead and ignores the indices.
 *
 * Generated in bulk from the curated map TwinModel/ue/assets/sign_atlas_cells.yaml by
 * TwinModel/ue/gen_sign_dataassets.py; do not hand-edit the generated assets.
 */
UCLASS(BlueprintType)
class CARLATOOLS_API USignDataAsset : public UPrimaryDataAsset
{
  GENERATED_BODY()

public:
  /// Plate mesh (one of Carla/Static/Signs/SignShapes).
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sign")
  UStaticMesh* SignMesh = nullptr;

  /// True when Diffuse is a standalone (non-atlas) texture; Id_X / Id_Y are then ignored.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sign")
  bool Unique = false;

  /// Atlas (or standalone) diffuse texture. The material derives the normal map from it.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sign")
  UTexture2D* Diffuse = nullptr;

  /// 1-based atlas column (left to right).
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sign", meta = (ClampMin = "1", ClampMax = "4"))
  int Id_X = 1;

  /// 1-based atlas row (top to bottom).
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sign", meta = (ClampMin = "1", ClampMax = "4"))
  int Id_Y = 1;

  /// Ready-to-use material for the plate (a constant instance of the atlas selector with the
  /// cell baked in). Runtime actors reference this instead of building a dynamic instance.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sign")
  UMaterialInterface* Material = nullptr;

  /// Regional style the plate follows.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
  ESignStyle Style = ESignStyle::None;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
  ESignCategory Category = ESignCategory::None;

  /// Catalog identifier, unique within a style (snake_case, e.g. "max_speed_30").
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
  FString SignName;

  /// What a person recognises the sign by.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
  FString Description;

  /// Code of the sign in its convention (Vienna "B2a", MUTCD "R1-1", GB 5768), may be empty.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
  FString Meaning;

  /// OpenDRIVE signal type this sign renders (CARLA / StVO vocabulary, e.g. "206" stop,
  /// "274" maximum speed), empty when the sign has no OpenDRIVE counterpart.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
  FString XodrType;

  /// OpenDRIVE signal subtype (the speed for "274"), "-1" or empty otherwise.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
  FString XodrSubtype;

  /// OpenStreetMap tags ("key=value") a node would carry for this sign; the generator
  /// matches them exactly.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
  TArray<FString> OsmTags;

  /// True when this asset matches an OSM node carrying Key=Value.
  UFUNCTION(BlueprintCallable, Category = "Catalog")
  bool MatchesOsmTag(const FString& Key, const FString& Value) const
  {
    return OsmTags.Contains(Key + TEXT("=") + Value);
  }

  /// True when this asset renders the given OpenDRIVE signal (type + subtype).
  UFUNCTION(BlueprintCallable, Category = "Catalog")
  bool MatchesXodrSignal(const FString& Type, const FString& Subtype) const
  {
    if (XodrType.IsEmpty() || XodrType != Type)
    {
      return false;
    }
    return XodrSubtype.IsEmpty() || XodrSubtype == TEXT("-1") || XodrSubtype == Subtype;
  }
};
