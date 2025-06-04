// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderLightScene.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"


void CarlaRecorderLightScene::Write(std::ostream &OutFile)
{
  WriteValue<int>(OutFile, this->LightId);
  WriteValue<float>(OutFile, this->Intensity);
  WriteValue<FLinearColor>(OutFile, this->Color);
  WriteValue<bool>(OutFile, this->bOn);
  WriteValue<uint8>(OutFile, this->Type);
}
void CarlaRecorderLightScene::Read(std::istream &InFile)
{
  ReadValue<int>(InFile, this->LightId);
  ReadValue<float>(InFile, this->Intensity);
  ReadValue<FLinearColor>(InFile, this->Color);
  ReadValue<bool>(InFile, this->bOn);
  ReadValue<uint8>(InFile, this->Type);
}

// ---------------------------------------------

void CarlaRecorderLightScenes::Clear(void)
{
  Lights.clear();
}

void CarlaRecorderLightScenes::Add(const CarlaRecorderLightScene &Vehicle)
{
  Lights.push_back(Vehicle);
}

void CarlaRecorderLightScenes::Write(std::ostream &OutFile)
{
  if (Lights.size() == 0)
  {
    return;
  }
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::SceneLight));

  std::streampos PosStart = OutFile.tellp();

  // write a dummy packet size
  uint32_t Total = 2 + Lights.size() * sizeof(CarlaRecorderLightScene);
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = Lights.size();
  WriteValue<uint16_t>(OutFile, Total);

  for (auto& Light : Lights)
  {
    Light.Write(OutFile);
  }

}

void CarlaRecorderLightScenes::Read(std::istream &InFile)
{
  uint16_t Total;
  CarlaRecorderLightScene LightScene;

  // read Total light events
  ReadValue<uint16_t>(InFile, Total);
  for (uint16_t i = 0; i < Total; ++i)
  {
    LightScene.Read(InFile);
    Add(LightScene);
  }
}

const std::vector<CarlaRecorderLightScene>& CarlaRecorderLightScenes::GetLights()
{
  return Lights;
}
