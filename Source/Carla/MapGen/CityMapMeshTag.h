// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

/// Tag to identify the meshes used by the ProceduralMapGenerator.
///
/// It will work as long as we have less than 255 meshes, currently blueprint
/// type enums support uint8 only.
UENUM(BlueprintType)
enum class ECityMapMeshTag : uint8
{
  RoadTwoLanes_LaneLeft               UMETA(DisplayName = "Road: Two Lanes - Lane Left"),
  RoadTwoLanes_LaneRight              UMETA(DisplayName = "Road: Two Lanes - Lane Right"),
  RoadTwoLanes_SidewalkLeft           UMETA(DisplayName = "Road: Two Lanes - Sidewalk Left"),
  RoadTwoLanes_SidewalkRight          UMETA(DisplayName = "Road: Two Lanes - Sidewalk Right"),

  Road90DegTurn_Lane0                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 0"),
  Road90DegTurn_Lane1                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 1"),
  Road90DegTurn_Lane2                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 2"),
  Road90DegTurn_Lane3                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 3"),
  Road90DegTurn_Sidewalk0             UMETA(DisplayName = "Road: 90 Degree Turn - Sidewalk 0"),
  Road90DegTurn_Sidewalk1             UMETA(DisplayName = "Road: 90 Degree Turn - Sidewalk 1"),
  Road90DegTurn_Sidewalk2             UMETA(DisplayName = "Road: 90 Degree Turn - Sidewalk 2"),
  Road90DegTurn_Sidewalk3             UMETA(DisplayName = "Road: 90 Degree Turn - Sidewalk 3"),

  RoadTIntersection_Lane0             UMETA(DisplayName = "Road: T-Intersection - Lane 0"),
  RoadTIntersection_Lane1             UMETA(DisplayName = "Road: T-Intersection - Lane 1"),
  RoadTIntersection_Lane2             UMETA(DisplayName = "Road: T-Intersection - Lane 2"),
  RoadTIntersection_Lane3             UMETA(DisplayName = "Road: T-Intersection - Lane 3"),
  RoadTIntersection_Sidewalk0         UMETA(DisplayName = "Road: T-Intersection - Sidewalk 0"),
  RoadTIntersection_Sidewalk1         UMETA(DisplayName = "Road: T-Intersection - Sidewalk 1"),
  RoadTIntersection_Sidewalk2         UMETA(DisplayName = "Road: T-Intersection - Sidewalk 2"),
  RoadTIntersection_Sidewalk3         UMETA(DisplayName = "Road: T-Intersection - Sidewalk 3"),

  RoadXIntersection_Lane0             UMETA(DisplayName = "Road: X-Intersection - Lane 0"),
  RoadXIntersection_Lane1             UMETA(DisplayName = "Road: X-Intersection - Lane 1"),
  RoadXIntersection_Lane2             UMETA(DisplayName = "Road: X-Intersection - Lane 2"),
  RoadXIntersection_Lane3             UMETA(DisplayName = "Road: X-Intersection - Lane 3"),
  RoadXIntersection_Sidewalk0         UMETA(DisplayName = "Road: X-Intersection - Sidewalk 0"),
  RoadXIntersection_Sidewalk1         UMETA(DisplayName = "Road: X-Intersection - Sidewalk 1"),
  RoadXIntersection_Sidewalk2         UMETA(DisplayName = "Road: X-Intersection - Sidewalk 2"),
  RoadXIntersection_Sidewalk3         UMETA(DisplayName = "Road: X-Intersection - Sidewalk 3"),

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
