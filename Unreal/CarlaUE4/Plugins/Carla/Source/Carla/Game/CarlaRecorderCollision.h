// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <vector>
#include <unordered_set>

struct CarlaRecorderCollision
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
    bool operator==(const CarlaRecorderCollision &Other) const;
};

// implement the hash function for the unordered_set of collisions
namespace std
{
    template<>
    struct hash<CarlaRecorderCollision>
    {
        std::size_t operator()(const CarlaRecorderCollision& P) const noexcept
        {
            return (P.DatabaseId1 * 100000) + P.DatabaseId2;
        }
    };
}

class CarlaRecorderCollisions{

    public:
    void Add(const CarlaRecorderCollision &Collision);
    void Clear(void);
    void Write(std::ofstream &OutFile);

    private:
    std::unordered_set<CarlaRecorderCollision> Collisions;
};
