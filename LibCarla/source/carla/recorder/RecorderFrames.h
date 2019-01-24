// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <chrono>

namespace carla {
namespace recorder {

#pragma pack(push, 1)
struct RecorderFrame {
    unsigned long id;
    double durationThis;
    double elapsed;

    void read(std::ifstream &file);
    void write(std::ofstream &file);
};
#pragma pack(pop)

class RecorderFrames {

    public:
    RecorderFrames(void);
    void setFrame(void);
    void write(std::ofstream &file, std::ofstream &log);
    

    private:
    RecorderFrame frame;
    unsigned long offsetPreviousFrame;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
};

}
}