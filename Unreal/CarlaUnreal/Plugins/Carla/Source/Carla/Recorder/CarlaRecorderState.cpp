// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderState.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

#include <algorithm>
#include <limits>

void CarlaRecorderStateTrafficLight::Write(std::ostream &OutFile)
{
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  WriteValue<bool>(OutFile, this->IsFrozen);
  WriteValue<float>(OutFile, this->ElapsedTime);
  WriteValue<char>(OutFile, this->State);
}

void CarlaRecorderStateTrafficLight::Read(std::istream &InFile)
{
  ReadValue<uint32_t>(InFile, this->DatabaseId);
  ReadValue<bool>(InFile, this->IsFrozen);
  ReadValue<float>(InFile, this->ElapsedTime);
  ReadValue<char>(InFile, this->State);
}

// ---------------------------------------------

void CarlaRecorderStates::Clear(void)
{
  StatesTrafficLights.clear();
}

void CarlaRecorderStates::Add(const CarlaRecorderStateTrafficLight &State)
{
  StatesTrafficLights.push_back(std::move(State));
}

void CarlaRecorderStates::Write(std::ostream &OutFile)
{
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::State));

  // a uint16 count of records follows; never write more than it can hold
  const uint16_t Count = std::min<size_t>(StatesTrafficLights.size(), std::numeric_limits<uint16_t>::max());

  // write the packet size
  uint32_t Total = 2 + Count * sizeof(CarlaRecorderStateTrafficLight);
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = Count;
  WriteValue<uint16_t>(OutFile, Total);

  for (uint16_t i = 0; i < Total; ++i)
  {
    StatesTrafficLights[i].Write(OutFile);
  }
}

void CarlaRecorderStates::Read(std::istream &InFile)
{
  uint16_t i, Total = 0;
  CarlaRecorderStateTrafficLight StateTrafficLight;

  // read Total traffic light states
  ReadValue<uint16_t>(InFile, Total);
  for (i = 0; (i < Total) && InFile; ++i)
  {
    StateTrafficLight.Read(InFile);
    Add(StateTrafficLight);
  }
}

const std::vector<CarlaRecorderStateTrafficLight>& CarlaRecorderStates::GetStates()
{
  return StatesTrafficLights;
}
