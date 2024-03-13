// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <sstream>
#include <vector>

struct CarlaRecorderEventDel
{
    uint32_t DatabaseId;

    void Read(std::istream &InFile);
    void Write(std::ostream &OutFile) const;
};

class CarlaRecorderEventsDel
{

    public:
    void Add(const CarlaRecorderEventDel &Event);
    void Clear(void);
    void Write(std::ostream &OutFile);
    void Read(std::istream &InFile);
    const std::vector<CarlaRecorderEventDel>& GetEvents();

    private:
    std::vector<CarlaRecorderEventDel> Events;
};
