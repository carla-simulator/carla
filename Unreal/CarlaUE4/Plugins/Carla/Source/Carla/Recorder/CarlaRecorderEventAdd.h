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
    uint8_t Type;
    FVector Location;
    FVector Rotation;
    CarlaRecorderActorDescription Description;

    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
};

class CarlaRecorderEventsAdd
{

    public:
    void Add(const CarlaRecorderEventAdd &Event);
    void Clear(void);
    void Write(std::ofstream &OutFile);

    private:
    std::vector<CarlaRecorderEventAdd> Events;
};
