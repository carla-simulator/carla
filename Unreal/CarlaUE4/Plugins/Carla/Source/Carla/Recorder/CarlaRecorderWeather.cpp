// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderWeather.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderWeather::Write(std::ostream &OutFile)
{
  WriteValue(OutFile, this->Cloudiness);
  WriteValue(OutFile, this->Precipitation);
  WriteValue(OutFile, this->PrecipitationDeposits);
  WriteValue(OutFile, this->WindIntensity);
  WriteValue(OutFile, this->SunAzimuthAngle);
  WriteValue(OutFile, this->SunAltitudeAngle);
  WriteValue(OutFile, this->FogDensity);
  WriteValue(OutFile, this->FogDistance);
  WriteValue(OutFile, this->FogFalloff);
  WriteValue(OutFile, this->Wetness);
  WriteValue(OutFile, this->ScatteringIntensity);
  WriteValue(OutFile, this->MieScatteringScale);
  WriteValue(OutFile, this->RayleighScatteringScale);
  WriteValue(OutFile, this->DustStorm);
}

void CarlaRecorderWeather::Read(std::istream &InFile)
{
  ReadValue(InFile, this->Cloudiness);
  ReadValue(InFile, this->Precipitation);
  ReadValue(InFile, this->PrecipitationDeposits);
  ReadValue(InFile, this->WindIntensity);
  ReadValue(InFile, this->SunAzimuthAngle);
  ReadValue(InFile, this->SunAltitudeAngle);
  ReadValue(InFile, this->FogDensity);
  ReadValue(InFile, this->FogDistance);
  ReadValue(InFile, this->FogFalloff);
  ReadValue(InFile, this->Wetness);
  ReadValue(InFile, this->ScatteringIntensity);
  ReadValue(InFile, this->MieScatteringScale);
  ReadValue(InFile, this->RayleighScatteringScale);
  ReadValue(InFile, this->DustStorm);
}

// ---------------------------------------------

void CarlaRecorderWeathers::Clear(void)
{
  Weathers.clear();
}

void CarlaRecorderWeathers::Add(const CarlaRecorderWeather &InObj)
{
  Weathers.push_back(InObj);
}

void CarlaRecorderWeathers::Write(std::ostream &OutFile)
{
  if (Weathers.size() == 0)
  {
    return;
  }

  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::Weather));

  // write the packet size
  uint32_t Total = 2 + Weathers.size() * sizeof(CarlaRecorderWeather);
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = Weathers.size();
  WriteValue<uint16_t>(OutFile, Total);

  // write records
  for (auto& Weather : Weathers)
  {
    Weather.Write(OutFile);
  }
}
