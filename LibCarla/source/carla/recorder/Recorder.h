// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include "carla/recorder/RecorderFrames.h"
#include "carla/recorder/RecorderEvent.h"
#include "carla/recorder/RecorderPosition.h"
#include "carla/recorder/RecorderHelpers.h"

namespace carla {
namespace recorder {


  enum class RecorderPacketId : char {
    Frame = 0,
    Event,
    Position,
    State
  };

  class Recorder {
  
    bool enabled;   // enabled or not

    // files
    std::ofstream file;
    std::ofstream log;

    // structures
    RecorderFrames frames;
    RecorderEvents events;
    RecorderPositions positions;

    public:

    Recorder();
    ~Recorder();
    void enable(void);
    void disable(void);
    bool isEnabled(void) { return enabled; };
    std::string start(std::string path = ".", std::string _map = "");
    void stop(void);
    void clear(void);
    void addEvent(const RecorderEvent &_event);
    void addPosition(const RecorderPosition &_position);
    void write(void);

  };

}
}