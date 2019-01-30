// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "RecorderHelpers.h"

#include <fstream>
#include <ctime>

namespace carla {
namespace recorder {

#pragma pack(push, 1)
struct RecorderInfo {
    short version;
    Buffer magic;
    std::time_t date;
    Buffer mapfile;

void read(std::ifstream &file) {
    readValue<short>(file, version);
    readBuffer(file, magic);
    readValue<std::time_t>(file, date);
    readBuffer(file, mapfile);
}

void write(std::ofstream &file) {
    writeValue<short>(file, version);
    writeBuffer(file, magic);
    writeValue<std::time_t>(file, date);
    writeBuffer(file, mapfile);
}
};
#pragma pack(pop)

}
}