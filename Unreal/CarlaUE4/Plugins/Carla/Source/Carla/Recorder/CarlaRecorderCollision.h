// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <vector>
#include <unordered_set>

#pragma pack(push, 1)
struct CarlaRecorderCollision
{
    uint32_t Id;
    uint32_t DatabaseId1;
    uint32_t DatabaseId2;
    bool IsActor1Hero;
    bool IsActor2Hero;

    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
    // define operator == needed for the 'unordered_set'
    bool operator==(const CarlaRecorderCollision &Other) const;
};
#pragma pack(pop)

// implement the hash function for the unordered_set of collisions
namespace std
{
    template<>
    struct hash<CarlaRecorderCollision>
    {
        std::size_t operator()(const CarlaRecorderCollision& P) const noexcept
        {
            std::size_t hash = P.DatabaseId1;
            hash <<= 32;
            hash += P.DatabaseId2;
            return hash;
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
