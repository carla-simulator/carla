// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <vector>

struct CarlaRecorderActorAttribute
{
    uint8_t Type;       // EActorAttributeType
    FString Id;         // string
    FString Value;      // string
};

struct CarlaRecorderActorDescription
{
    uint32_t UId;
    FString Id;          // string
    std::vector<CarlaRecorderActorAttribute> Attributes;
};

struct CarlaRecorderEventAdd
{
    uint32_t DatabaseId;
    FVector Location;
    FVector Rotation;
    CarlaRecorderActorDescription Description;

    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile);
};

struct CarlaRecorderEventDel
{
    uint32_t DatabaseId;

    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile);
};

struct CarlaRecorderEventParent
{
    uint32_t DatabaseId;
    uint32_t DatabaseIdParent;

    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile);
};

class CarlaRecorderEvents {

    public:
    void AddEvent(const CarlaRecorderEventAdd &Event);
    void AddEvent(const CarlaRecorderEventDel &Event);
    void AddEvent(const CarlaRecorderEventParent &Event);
    void Clear(void);
    void Write(std::ofstream &OutFile, std::ofstream &OutLog);

    private:
    std::vector<CarlaRecorderEventAdd> EventsAdd;
    std::vector<CarlaRecorderEventDel> EventsDel;
    std::vector<CarlaRecorderEventParent> EventsParent;

    void WriteEventsAdd(std::ofstream &OutFile);
    void WriteEventsDel(std::ofstream &OutFile, std::ofstream &OutLog);
    void WriteEventsParent(std::ofstream &OutFile, std::ofstream &OutLog);
};
