// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorder.h"
#include "CarlaRecorderEvent.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderEventAdd::Write(std::ofstream &OutFile) const
{
    // OutLog << "add event: " << TCHAR_TO_UTF8(this->Description.Id) << " (id." << this->DatabaseId << ")\n";

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
    // OutLog << "Attributes: " << this->description.attributes.size() << std::endl;
    for (uint16_t i=0; i<Total; ++i)
    {
        // type
        WriteValue<uint8_t>(OutFile, this->Description.Attributes[i].Type);
        WriteFString(OutFile, this->Description.Attributes[i].Id);
        WriteFString(OutFile, this->Description.Attributes[i].Value);
        // log
        // OutLog << "  " << TCHAR_TO_UTF8(this->Description.Attributes[i].Id) << " = " << TCHAR_TO_UTF8(this->Description.Attributes[i].Value) << std::endl;
    }
}

void CarlaRecorderEventAdd::Read(std::ifstream &InFile)
{
    //OutLog << "add event: " << this->description.id.c_str() << " (id." << this->databaseId << ")\n";
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
    //OutLog << "Attributes: " << this->description.attributes.size() << std::endl;
    for (uint16_t i=0; i<Total; ++i)
    {
        CarlaRecorderActorAttribute Att;
        ReadValue<uint8_t>(InFile, Att.Type);
        ReadFString(InFile, Att.Id);
        ReadFString(InFile, Att.Value);
        this->Description.Attributes.push_back(std::move(Att));
        // log
        //OutLog << "  " << att.id.c_str() << " = " << att.value.c_str() << std::endl;
    }
}

void CarlaRecorderEventDel::Read(std::ifstream &InFile)
{
    // database id
    ReadValue<uint32_t>(InFile, this->DatabaseId);
}
void CarlaRecorderEventDel::Write(std::ofstream &OutFile) const
{
    // database id
    WriteValue<uint32_t>(OutFile, this->DatabaseId);
}

void CarlaRecorderEventParent::Read(std::ifstream &InFile)
{
    // database id
    ReadValue<uint32_t>(InFile, this->DatabaseId);
    // database id parent
    ReadValue<uint32_t>(InFile, this->DatabaseIdParent);
}
void CarlaRecorderEventParent::Write(std::ofstream &OutFile) const
{
    // database id
    WriteValue<uint32_t>(OutFile, this->DatabaseId);
    // database id parent
    WriteValue<uint32_t>(OutFile, this->DatabaseIdParent);
}

void CarlaRecorderEventCollision::Read(std::ifstream &InFile)
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
void CarlaRecorderEventCollision::Write(std::ofstream &OutFile) const
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
bool CarlaRecorderEventCollision::operator==(const CarlaRecorderEventCollision &Other) const
{
    return (this->DatabaseId1 == Other.DatabaseId1 &&
            this->DatabaseId2 == Other.DatabaseId2);
}
//---------------------------------------------

void CarlaRecorderEvents::Clear(void)
{
    EventsAdd.clear();
    EventsDel.clear();
    EventsParent.clear();
    EventsCollision.clear();
}

void CarlaRecorderEvents::AddEvent(const CarlaRecorderEventAdd &Event)
{
    EventsAdd.push_back(std::move(Event));
}

void CarlaRecorderEvents::AddEvent(const CarlaRecorderEventDel &Event)
{
    EventsDel.push_back(std::move(Event));
}

void CarlaRecorderEvents::AddEvent(const CarlaRecorderEventParent &Event)
{
    EventsParent.push_back(std::move(Event));
}

void CarlaRecorderEvents::AddEvent(const CarlaRecorderEventCollision &Event)
{
    EventsCollision.insert(std::move(Event));
}

void CarlaRecorderEvents::WriteEventsAdd(std::ofstream &OutFile)
{
    // write total records
    uint16_t Total = EventsAdd.size();
    WriteValue<uint16_t>(OutFile, Total);

    for (uint16_t i=0; i<Total; ++i)
        EventsAdd[i].Write(OutFile);
}

void CarlaRecorderEvents::WriteEventsDel(std::ofstream &OutFile, std::ofstream &OutLog)
{

    // write total records
    uint16_t Total = EventsDel.size();
    WriteValue<uint16_t>(OutFile, Total);

    for (uint16_t i=0; i<Total; ++i)
    {
        OutLog << "add del: " << EventsDel[i].DatabaseId << "\n";
        EventsDel[i].Write(OutFile);
    }
}

void CarlaRecorderEvents::WriteEventsParent(std::ofstream &OutFile, std::ofstream &OutLog)
{
    // write total records
    uint16_t Total = EventsParent.size();
    WriteValue<uint16_t>(OutFile, Total);

    for (uint16_t i=0; i<Total; ++i)
    {
        OutLog << "add parent: id." << EventsParent[i].DatabaseId << ", parent." << EventsParent[i].DatabaseIdParent << "\n";
        EventsParent[i].Write(OutFile);
    }
}

void CarlaRecorderEvents::WriteEventsCollision(std::ofstream &OutFile, std::ofstream &OutLog)
{
    // write total records
    uint16_t Total = EventsCollision.size();
    WriteValue<uint16_t>(OutFile, Total);

    // for (uint16_t i=0; i<Total; ++i)
    for (auto &Coll : EventsCollision)
    {
        // OutLog << "add collision: id." << EventsCollision[i].Id << " actor1." << EventsCollision[i].DatabaseId1 << " actor2." << EventsCollision[i].DatabaseId2 << "\n";
        // EventsCollision[i].Write(OutFile);
        OutLog << "add collision: id." << Coll.Id << " actor1." << Coll.DatabaseId1 << " actor2." << Coll.DatabaseId2 << "\n";
        Coll.Write(OutFile);
        UE_LOG(LogCarla, Warning, TEXT("Collision"));
    }
}

void CarlaRecorderEvents::Write(std::ofstream &OutFile, std::ofstream &OutLog)
{
    // write the packet id
    WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::Event));

    std::streampos PosStart = OutFile.tellp();

    // write a dummy packet size
    uint32_t Total = 0;
    WriteValue<uint32_t>(OutFile, Total);

    // write events
    WriteEventsAdd(OutFile);
    WriteEventsDel(OutFile, OutLog);
    WriteEventsParent(OutFile, OutLog);
    WriteEventsCollision(OutFile, OutLog);

    // write the real packet size
    std::streampos PosEnd = OutFile.tellp();
    Total = PosEnd - PosStart - sizeof(uint32_t);
    OutFile.seekp(PosStart, std::ios::beg);
    WriteValue<uint32_t>(OutFile, Total);
    OutFile.seekp(PosEnd, std::ios::beg);

    OutLog << "write events (" << EventsAdd.size() << "," << EventsDel.size() << "," << EventsParent.size() << ")\n";
}