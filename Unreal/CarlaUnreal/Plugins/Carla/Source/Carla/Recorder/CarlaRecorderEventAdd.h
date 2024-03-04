// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <sstream>
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

    void Read(std::istream &InFile);
    void Write(std::ostream &OutFile) const;
};

class CarlaRecorderEventsAdd
{

    public:
    void Add(const CarlaRecorderEventAdd &Event);
    void Clear(void);
    void Write(std::ostream &OutFile);
    void Read(std::istream &InFile);
    const std::vector<CarlaRecorderEventAdd>& GetEvents();

    private:
    std::vector<CarlaRecorderEventAdd> Events;
};
