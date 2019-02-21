// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <vector>
#include <unordered_set>

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
    uint8_t Type;
    FVector Location;
    FVector Rotation;
    CarlaRecorderActorDescription Description;

    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
};

struct CarlaRecorderEventDel
{
    uint32_t DatabaseId;

    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
};

struct CarlaRecorderEventParent
{
    uint32_t DatabaseId;
    uint32_t DatabaseIdParent;

    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
};

struct CarlaRecorderEventCollision
{
    uint32_t Id;
    uint32_t DatabaseId1;
    uint32_t DatabaseId2;
    bool IsActor1Hero;
    bool IsActor2Hero;
    FVector Location;

    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
    // define operator == needed for the 'unordered_set'
    bool operator==(const CarlaRecorderEventCollision &Other) const;
};

// implement the hash function for the unordered_set of collisions
namespace std
{
    template<>
    struct hash<CarlaRecorderEventCollision>
    {
        std::size_t operator()(const CarlaRecorderEventCollision& P) const noexcept
        {
            return (P.DatabaseId1 * 100000) + P.DatabaseId2;
        }
    };
}

class CarlaRecorderEvents
{

    public:
    void AddEvent(const CarlaRecorderEventAdd &Event);
    void AddEvent(const CarlaRecorderEventDel &Event);
    void AddEvent(const CarlaRecorderEventParent &Event);
    void AddEvent(const CarlaRecorderEventCollision &Event);
    void Clear(void);
    void Write(std::ofstream &OutFile);

    private:
    std::vector<CarlaRecorderEventAdd> EventsAdd;
    std::vector<CarlaRecorderEventDel> EventsDel;
    std::vector<CarlaRecorderEventParent> EventsParent;
    std::unordered_set<CarlaRecorderEventCollision> EventsCollision;

    void WriteEventsAdd(std::ofstream &OutFile);
    void WriteEventsDel(std::ofstream &OutFile);
    void WriteEventsParent(std::ofstream &OutFile);
    void WriteEventsCollision(std::ofstream &OutFile);
};
