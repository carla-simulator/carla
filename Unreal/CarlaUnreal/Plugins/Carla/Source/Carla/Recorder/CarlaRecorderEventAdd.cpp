// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderEventAdd.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderEventAdd::Write(std::ostream &OutFile) const
{
    // database id
    WriteValue<uint32_t>(OutFile, this->DatabaseId);
    WriteValue<uint8_t>(OutFile, this->Type);

    // transform
    WriteFVector(OutFile, this->Location);
    WriteFVector(OutFile, this->Rotation);

    // description type
    WriteValue<uint32_t>(OutFile, this->Description.UId);
    WriteFString(OutFile, this->Description.Id);

    // attributes
    uint16_t Total = this->Description.Attributes.size();
    WriteValue<uint16_t>(OutFile, Total);
    for (uint16_t i=0; i<Total; ++i)
    {
        // type
        WriteValue<uint8_t>(OutFile, this->Description.Attributes[i].Type);
        WriteFString(OutFile, this->Description.Attributes[i].Id);
        WriteFString(OutFile, this->Description.Attributes[i].Value);
    }
}

void CarlaRecorderEventAdd::Read(std::istream &InFile)
{
    // database id
    ReadValue<uint32_t>(InFile, this->DatabaseId);

    // database type
    ReadValue<uint8_t>(InFile, this->Type);

    // transform
    ReadFVector(InFile, this->Location);
    ReadFVector(InFile, this->Rotation);

    // description type
    ReadValue<uint32_t>(InFile, this->Description.UId);
    ReadFString(InFile, this->Description.Id);

    // attributes
    uint16_t Total;
    ReadValue<uint16_t>(InFile, Total);
    this->Description.Attributes.clear();
    this->Description.Attributes.reserve(Total);
    for (uint16_t i=0; i<Total; ++i)
    {
        CarlaRecorderActorAttribute Att;
        ReadValue<uint8_t>(InFile, Att.Type);
        ReadFString(InFile, Att.Id);
        ReadFString(InFile, Att.Value);
        this->Description.Attributes.push_back(std::move(Att));
    }
}

//---------------------------------------------

void CarlaRecorderEventsAdd::Clear(void)
{
    Events.clear();
}

void CarlaRecorderEventsAdd::Add(const CarlaRecorderEventAdd &Event)
{
    Events.push_back(std::move(Event));
}

void CarlaRecorderEventsAdd::Write(std::ostream &OutFile)
{
    // write the packet id
    WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::EventAdd));

    std::streampos PosStart = OutFile.tellp();

    // write a dummy packet size
    uint32_t Total = 0;
    WriteValue<uint32_t>(OutFile, Total);

    // write total records
    Total = Events.size();
    WriteValue<uint16_t>(OutFile, Total);

    for (uint16_t i=0; i<Total; ++i)
        Events[i].Write(OutFile);

    // write the real packet size
    std::streampos PosEnd = OutFile.tellp();
    Total = PosEnd - PosStart - sizeof(uint32_t);
    OutFile.seekp(PosStart, std::ios::beg);
    WriteValue<uint32_t>(OutFile, Total);
    OutFile.seekp(PosEnd, std::ios::beg);
}

void CarlaRecorderEventsAdd::Read(std::istream &InFile)
{
    CarlaRecorderEventAdd EventAdd;
    uint16_t i, Total;
    ReadValue<uint16_t>(InFile, Total);
    for (i = 0; i < Total; ++i)
    {
        EventAdd.Read(InFile);
        Add(EventAdd);
    }
}

const std::vector<CarlaRecorderEventAdd>& CarlaRecorderEventsAdd::GetEvents()
{
    return Events;
}
