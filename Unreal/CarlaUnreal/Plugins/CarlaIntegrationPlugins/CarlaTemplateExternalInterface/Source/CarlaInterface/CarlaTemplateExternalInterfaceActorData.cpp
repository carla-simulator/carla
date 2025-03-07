// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaTemplateExternalInterfaceActorData.h"

void FCarlaTemplateExternalInterfaceData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(CarlaActor, CarlaEpisode);
}

void FCarlaTemplateExternalInterfaceData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RestoreActorData(CarlaActor, CarlaEpisode);
}
