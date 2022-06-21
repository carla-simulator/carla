// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

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
  RoadTwoLanes_LaneMarkingSolid       UMETA(DisplayName = "Road: Two Lanes - Lane Marking Solid"),
  RoadTwoLanes_LaneMarkingBroken      UMETA(DisplayName = "Road: Two Lanes - Lane Marking Broken"),

  Road90DegTurn_Lane0                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 0"),
  Road90DegTurn_Lane1                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 1"),
  Road90DegTurn_Lane2                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 2"),
  Road90DegTurn_Lane3                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 3"),
  Road90DegTurn_Lane4                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 4"),
  Road90DegTurn_Lane5                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 5"),
  Road90DegTurn_Lane6                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 6"),
  Road90DegTurn_Lane7                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 7"),
  Road90DegTurn_Lane8                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 8"),
  Road90DegTurn_Lane9                 UMETA(DisplayName = "Road: 90 Degree Turn - Lane 9"),
  Road90DegTurn_Sidewalk0             UMETA(DisplayName = "Road: 90 Degree Turn - Sidewalk 0"),
  Road90DegTurn_Sidewalk1             UMETA(DisplayName = "Road: 90 Degree Turn - Sidewalk 1"),
  Road90DegTurn_Sidewalk2             UMETA(DisplayName = "Road: 90 Degree Turn - Sidewalk 2"),
  Road90DegTurn_Sidewalk3             UMETA(DisplayName = "Road: 90 Degree Turn - Sidewalk 3"),
  Road90DegTurn_LaneMarking           UMETA(DisplayName = "Road: 90 Degree Turn - Lane Marking"),

  RoadTIntersection_Lane0             UMETA(DisplayName = "Road: T-Intersection - Lane 0"),
  RoadTIntersection_Lane1             UMETA(DisplayName = "Road: T-Intersection - Lane 1"),
  RoadTIntersection_Lane2             UMETA(DisplayName = "Road: T-Intersection - Lane 2"),
  RoadTIntersection_Lane3             UMETA(DisplayName = "Road: T-Intersection - Lane 3"),
  RoadTIntersection_Lane4             UMETA(DisplayName = "Road: T-Intersection - Lane 4"),
  RoadTIntersection_Lane5             UMETA(DisplayName = "Road: T-Intersection - Lane 5"),
  RoadTIntersection_Lane6             UMETA(DisplayName = "Road: T-Intersection - Lane 6"),
  RoadTIntersection_Lane7             UMETA(DisplayName = "Road: T-Intersection - Lane 7"),
  RoadTIntersection_Lane8             UMETA(DisplayName = "Road: T-Intersection - Lane 8"),
  RoadTIntersection_Lane9             UMETA(DisplayName = "Road: T-Intersection - Lane 9"),
  RoadTIntersection_Sidewalk0         UMETA(DisplayName = "Road: T-Intersection - Sidewalk 0"),
  RoadTIntersection_Sidewalk1         UMETA(DisplayName = "Road: T-Intersection - Sidewalk 1"),
  RoadTIntersection_Sidewalk2         UMETA(DisplayName = "Road: T-Intersection - Sidewalk 2"),
  RoadTIntersection_Sidewalk3         UMETA(DisplayName = "Road: T-Intersection - Sidewalk 3"),
  RoadTIntersection_LaneMarking       UMETA(DisplayName = "Road: T-Intersection - Lane Marking"),

  RoadXIntersection_Lane0             UMETA(DisplayName = "Road: X-Intersection - Lane 0"),
  RoadXIntersection_Lane1             UMETA(DisplayName = "Road: X-Intersection - Lane 1"),
  RoadXIntersection_Lane2             UMETA(DisplayName = "Road: X-Intersection - Lane 2"),
  RoadXIntersection_Lane3             UMETA(DisplayName = "Road: X-Intersection - Lane 3"),
  RoadXIntersection_Lane4             UMETA(DisplayName = "Road: X-Intersection - Lane 4"),
  RoadXIntersection_Lane5             UMETA(DisplayName = "Road: X-Intersection - Lane 5"),
  RoadXIntersection_Lane6             UMETA(DisplayName = "Road: X-Intersection - Lane 6"),
  RoadXIntersection_Lane7             UMETA(DisplayName = "Road: X-Intersection - Lane 7"),
  RoadXIntersection_Lane8             UMETA(DisplayName = "Road: X-Intersection - Lane 8"),
  RoadXIntersection_Lane9             UMETA(DisplayName = "Road: X-Intersection - Lane 9"),
  RoadXIntersection_Sidewalk0         UMETA(DisplayName = "Road: X-Intersection - Sidewalk 0"),
  RoadXIntersection_Sidewalk1         UMETA(DisplayName = "Road: X-Intersection - Sidewalk 1"),
  RoadXIntersection_Sidewalk2         UMETA(DisplayName = "Road: X-Intersection - Sidewalk 2"),
  RoadXIntersection_Sidewalk3         UMETA(DisplayName = "Road: X-Intersection - Sidewalk 3"),
  RoadXIntersection_LaneMarking       UMETA(DisplayName = "Road: X-Intersection - Lane Marking"),

  NUMBER_OF_TAGS         UMETA(Hidden),
  INVALID                UMETA(Hidden)
};

/// Helper class for working with ECityMapMeshTag.
class CityMapMeshTag
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
