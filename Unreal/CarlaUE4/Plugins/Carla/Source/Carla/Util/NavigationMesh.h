// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

class FNavigationMesh
{
public:

  /// Return the Navigation Mesh Binary associated to @a MapName, or empty if the such
  /// file wasn't serialized.
  static TArray<uint8> Load(FString MapName);
};
