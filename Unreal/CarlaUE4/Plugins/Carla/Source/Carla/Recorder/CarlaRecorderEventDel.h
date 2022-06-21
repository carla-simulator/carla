// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <vector>

struct CarlaRecorderEventDel
{
    uint32_t DatabaseId;

    void Read(std::ifstream &InFile);
    void Write(std::ofstream &OutFile) const;
};

class CarlaRecorderEventsDel
{

    public:
    void Add(const CarlaRecorderEventDel &Event);
    void Clear(void);
    void Write(std::ofstream &OutFile);

    private:
    std::vector<CarlaRecorderEventDel> Events;
};
