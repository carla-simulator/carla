// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderWeather.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"


void CarlaRecorderWeather::Write(std::ofstream &OutFile) const
{
  WriteValue<float>(OutFile, this->Params.Cloudiness);
  WriteValue<float>(OutFile, this->Params.Precipitation);
  WriteValue<float>(OutFile, this->Params.PrecipitationDeposits);
  WriteValue<float>(OutFile, this->Params.WindIntensity);
  WriteValue<float>(OutFile, this->Params.SunAzimuthAngle);
  WriteValue<float>(OutFile, this->Params.SunAltitudeAngle);
  WriteValue<float>(OutFile, this->Params.FogDensity);
  WriteValue<float>(OutFile, this->Params.FogDistance);
  WriteValue<float>(OutFile, this->Params.FogFalloff);
  WriteValue<float>(OutFile, this->Params.Wetness);
  WriteValue<float>(OutFile, this->Params.ScatteringIntensity);
  WriteValue<float>(OutFile, this->Params.MieScatteringScale);
  WriteValue<float>(OutFile, this->Params.RayleighScatteringScale);
}

void CarlaRecorderWeather::Read(std::ifstream &InFile)
{
  ReadValue<float>(InFile, this->Params.Cloudiness);
  ReadValue<float>(InFile, this->Params.Precipitation);
  ReadValue<float>(InFile, this->Params.PrecipitationDeposits);
  ReadValue<float>(InFile, this->Params.WindIntensity);
  ReadValue<float>(InFile, this->Params.SunAzimuthAngle);
  ReadValue<float>(InFile, this->Params.SunAltitudeAngle);
  ReadValue<float>(InFile, this->Params.FogDensity);
  ReadValue<float>(InFile, this->Params.FogDistance);
  ReadValue<float>(InFile, this->Params.FogFalloff);
  ReadValue<float>(InFile, this->Params.Wetness);
  ReadValue<float>(InFile, this->Params.ScatteringIntensity);
  ReadValue<float>(InFile, this->Params.MieScatteringScale);
  ReadValue<float>(InFile, this->Params.RayleighScatteringScale);
}

std::string CarlaRecorderWeather::Print() const
{
  std::ostringstream oss;
  oss << TCHAR_TO_UTF8(*Params.ToString());
  return oss.str();
}

// ---------------------------------------------

void CarlaRecorderWeathers::Clear(void)
{
  Weathers.clear();
}

void CarlaRecorderWeathers::Add(const CarlaRecorderWeather &Weather)
{
  Weathers.push_back(Weather);
}

void CarlaRecorderWeathers::Write(std::ofstream &OutFile) const
{
  if (Weathers.size() == 0)
  {
    return;
  }
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::Weather));

  std::streampos PosStart = OutFile.tellp();

  // write a dummy packet size
  uint32_t Total = 2 + Weathers.size() * sizeof(CarlaRecorderWeather);
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = Weathers.size();
  WriteValue<uint16_t>(OutFile, Total);

  for (auto& Weather : Weathers)
  {
    Weather.Write(OutFile);
  }

}
