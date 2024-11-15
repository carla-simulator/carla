// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <util/ue-header-guard-begin.h>
#include <Containers/UnrealString.h>
#include <util/ue-header-guard-end.h>

#if WITH_EDITORONLY_DATA

// NOTE: Assets imported from a map FBX will be classified for semantic
// segmentation as ROAD, ROADLINES AND TERRAIN based on the asset name
// defined in RoadRunner. These tags will be used for moving the meshes
// and for specifying the path to these meshes when spawning them in a world.
namespace SSTags {
  // Carla Semantic Segmentation Folder Tags
  static const FString ROAD       = TEXT("Road");
  static const FString ROADLINE   = TEXT("RoadLine");
  static const FString TERRAIN    = TEXT("Terrain");
  static const FString GRASS      = TEXT("Terrain");
  static const FString SIDEWALK   = TEXT("SideWalk");
  static const FString CURB       = TEXT("SideWalk");
  static const FString GUTTER     = TEXT("SideWalk");

  // RoadRunner Tags
  //
  // RoadRunner's mesh naming convention:
  // mapName_meshType_meshSubtype_layerNumberNode
  //
  // meshType is a larger geographical tag (e.g. "Road", "Terrain")
  // meshSubType is a denomination of the tag (e.g. "Road", "Gutter", "Ground")
  static const FString R_ROAD1     = TEXT("Road_Road");
  static const FString R_ROAD2     = TEXT("Roads_Road");
  static const FString R_GRASS1    = TEXT("Road_Grass");
  static const FString R_GRASS2    = TEXT("Roads_Grass");
  static const FString R_MARKING1  = TEXT("Road_Marking");
  static const FString R_MARKING2  = TEXT("Roads_Marking");
  static const FString R_SIDEWALK1 = TEXT("Road_Sidewalk");
  static const FString R_SIDEWALK2 = TEXT("Roads_Sidewalk");
  static const FString R_CURB1     = TEXT("Road_Curb");
  static const FString R_CURB2     = TEXT("Roads_Curb");
  static const FString R_GUTTER1   = TEXT("Road_Gutter");
  static const FString R_GUTTER2   = TEXT("Roads_Gutter");
  static const FString R_TERRAIN   = TEXT("Terrain");
}

#endif
