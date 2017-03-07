// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

/// Tag to identify the meshes used by the ProceduralMapGenerator.
///
/// It will work as long as we have less than 255 meshes, currently blueprint
/// type enums support uint8 only.
UENUM(BlueprintType)
enum class ECityMapMeshTag : uint8
{
  RoadTwoLanes           UMETA(DisplayName = "Road: Two Lanes"),
  RoadFourLanes          UMETA(DisplayName = "Road: Four Lanes"),
  Road90DegTurn          UMETA(DisplayName = "Road: 90 Degree Turn"),
  RoadTIntersection      UMETA(DisplayName = "Road: T-Intersection"),
  RoadXIntersection      UMETA(DisplayName = "Road: X-Intersection"),

  NUMBER_OF_TAGS         UMETA(Hidden)
};

/// Helper class for working with ECityMapMeshTag.
class CARLA_API CityMapMeshTag
{
public:

  /// Return the number of tags.
  static constexpr uint8 GetNumberOfTags() {
    return ToUInt(ECityMapMeshTag::NUMBER_OF_TAGS);
  }

  /// Return the base mesh. The base mesh defines the unit tile for map scaling.
  static ECityMapMeshTag GetBaseMeshTag();

  /// Get the size in tiles of a road intersection side. I.e., return N such NxN
  /// is the size of a road intersection piece.
  static uint32 GetRoadIntersectionSize();

  /// @name Tag conversions
  /// @{

  /// Convert @a Tag to an unsigned integer type.
  static constexpr uint8 ToUInt(ECityMapMeshTag Tag) {
    return static_cast<uint8>(Tag);
  }

  /// Convert an unsigned integer to a ECityMapMeshTag.
  static ECityMapMeshTag FromUInt(uint8 Value) {
    check(Value < GetNumberOfTags());
    return static_cast<ECityMapMeshTag>(Value);
  }

  /// Get @a Tag name as FString.
  static FString ToString(ECityMapMeshTag Tag);

  /// Convert @a Value to ECityMapMeshTag and get its name as FString.
  static FString ToString(uint8 Value) {
    return ToString(FromUInt(Value));
  }

  /// @}
};
