// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderCollision.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderCollision::Read(std::istream &InFile)
{
    // id
    ReadValue<uint32_t>(InFile, this->Id);
    // actors database id
    ReadValue<uint32_t>(InFile, this->DatabaseId1);
    ReadValue<uint32_t>(InFile, this->DatabaseId2);
    // is hero
    ReadValue<bool>(InFile, this->IsActor1Hero);
    ReadValue<bool>(InFile, this->IsActor2Hero);
}
void CarlaRecorderCollision::Write(std::ostream &OutFile) const
{
    // id
    WriteValue<uint32_t>(OutFile, this->Id);
    // actors database id
    WriteValue<uint32_t>(OutFile, this->DatabaseId1);
    WriteValue<uint32_t>(OutFile, this->DatabaseId2);
    // is hero
    WriteValue<bool>(OutFile, this->IsActor1Hero);
    WriteValue<bool>(OutFile, this->IsActor2Hero);
}
bool CarlaRecorderCollision::operator==(const CarlaRecorderCollision &Other) const
{
    return (this->DatabaseId1 == Other.DatabaseId1 &&
            this->DatabaseId2 == Other.DatabaseId2);
}
//---------------------------------------------

void CarlaRecorderCollisions::Clear(void)
{
    Collisions.clear();
}

void CarlaRecorderCollisions::Add(const CarlaRecorderCollision &Collision)
{
    Collisions.insert(std::move(Collision));
}

void CarlaRecorderCollisions::Write(std::ostream &OutFile)
{
    // write the packet id
    WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::Collision));

    // write the packet size
    uint32_t Total = 2 + Collisions.size() * sizeof(CarlaRecorderCollision);
    WriteValue<uint32_t>(OutFile, Total);

    // write total records
    Total = Collisions.size();
    WriteValue<uint16_t>(OutFile, Total);

    // for (uint16_t i=0; i<Total; ++i)
    for (auto &Coll : Collisions)
    {
        Coll.Write(OutFile);
    }
}
