// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Settings/EpisodeSettings.h"

class CARLA_API FCarlaStaticDelegates
{
public:

  DECLARE_MULTICAST_DELEGATE_OneParam(FOnEpisodeSettingsChange, const FEpisodeSettings &);
  static FOnEpisodeSettingsChange OnEpisodeSettingsChange;
};
