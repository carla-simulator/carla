// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderEventDel.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderEventDel::Read(std::istream &InFile)
{
    // database id
    ReadValue<uint32_t>(InFile, this->DatabaseId);
}
void CarlaRecorderEventDel::Write(std::ostream &OutFile) const
{
    // database id
    WriteValue<uint32_t>(OutFile, this->DatabaseId);
}

//---------------------------------------------

void CarlaRecorderEventsDel::Clear(void)
{
    Events.clear();
}

void CarlaRecorderEventsDel::Add(const CarlaRecorderEventDel &Event)
{
    Events.push_back(std::move(Event));
}

void CarlaRecorderEventsDel::Write(std::ostream &OutFile)
{
    // write the packet id
    WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::EventDel));

    std::streampos PosStart = OutFile.tellp();

    // write a dummy packet size
    uint32_t Total = 0;
    WriteValue<uint32_t>(OutFile, Total);

    // write total records
    Total = Events.size();
    WriteValue<uint16_t>(OutFile, Total);

    for (uint16_t i=0; i<Total; ++i)
    {
        Events[i].Write(OutFile);
    }

    // write the real packet size
    std::streampos PosEnd = OutFile.tellp();
    Total = PosEnd - PosStart - sizeof(uint32_t);
    OutFile.seekp(PosStart, std::ios::beg);
    WriteValue<uint32_t>(OutFile, Total);
    OutFile.seekp(PosEnd, std::ios::beg);
}

void CarlaRecorderEventsDel::Read(std::istream &InFile)
{
    uint16_t i, Total;
    CarlaRecorderEventDel EventDel;

    // process destroy events
    ReadValue<uint16_t>(InFile, Total);
    for (i = 0; i < Total; ++i)
    {
        EventDel.Read(InFile);
        Add(EventDel);
    }
}

const std::vector<CarlaRecorderEventDel>& CarlaRecorderEventsDel::GetEvents()
{
    return Events;
}
