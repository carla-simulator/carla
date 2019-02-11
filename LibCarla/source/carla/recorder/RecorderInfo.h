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

struct RecorderInfo {
    uint16_t version;
    Buffer magic;
    std::time_t date;
    Buffer mapfile;

void read(std::ifstream &file) {
    readValue<uint16_t>(file, version);
    readBuffer(file, magic);
    readValue<std::time_t>(file, date);
    readBuffer(file, mapfile);
}

void write(std::ofstream &file) {
    writeValue<uint16_t>(file, version);
    writeBuffer(file, magic);
    writeValue<std::time_t>(file, date);
    writeBuffer(file, mapfile);
}
};

}
}