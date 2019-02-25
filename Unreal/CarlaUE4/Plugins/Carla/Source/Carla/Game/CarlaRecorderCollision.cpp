// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorder.h"
#include "CarlaRecorderCollision.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderCollision::Read(std::ifstream &InFile)
{
    // id
    ReadValue<uint32_t>(InFile, this->Id);
    // actors database id
    ReadValue<uint32_t>(InFile, this->DatabaseId1);
    ReadValue<uint32_t>(InFile, this->DatabaseId2);
    // is hero
    ReadValue<bool>(InFile, this->IsActor1Hero);
    ReadValue<bool>(InFile, this->IsActor2Hero);
    // location
    ReadFVector(InFile, this->Location);
}
void CarlaRecorderCollision::Write(std::ofstream &OutFile) const
{
    // id
    WriteValue<uint32_t>(OutFile, this->Id);
    // actors database id
    WriteValue<uint32_t>(OutFile, this->DatabaseId1);
    WriteValue<uint32_t>(OutFile, this->DatabaseId2);
    // is hero
    WriteValue<bool>(OutFile, this->IsActor1Hero);
    WriteValue<bool>(OutFile, this->IsActor2Hero);
    // location
    WriteFVector(OutFile, this->Location);
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

void CarlaRecorderCollisions::Write(std::ofstream &OutFile)
{
    // write the packet id
    WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::Collision));

    std::streampos PosStart = OutFile.tellp();

    // write a dummy packet size
    uint32_t Total = 0;
    WriteValue<uint32_t>(OutFile, Total);

    // write total records
    Total = Collisions.size();
    WriteValue<uint16_t>(OutFile, Total);

    // for (uint16_t i=0; i<Total; ++i)
    for (auto &Coll : Collisions)
    {
        Coll.Write(OutFile);
    }

    // write the real packet size
    std::streampos PosEnd = OutFile.tellp();
    Total = PosEnd - PosStart - sizeof(uint32_t);
    OutFile.seekp(PosStart, std::ios::beg);
    WriteValue<uint32_t>(OutFile, Total);
    OutFile.seekp(PosEnd, std::ios::beg);
}