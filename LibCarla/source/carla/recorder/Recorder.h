// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include "carla/NonCopyable.h"
#include "carla/recorder/RecorderInfo.h"
#include "carla/recorder/RecorderFrames.h"
#include "carla/recorder/RecorderEvent.h"
#include "carla/recorder/RecorderPosition.h"
#include "carla/recorder/RecorderHelpers.h"
#include "carla/recorder/Replayer.h"

namespace carla {
namespace recorder {

  enum class RecorderPacketId : char {
    Frame = 0,
    Event,
    Position,
    State
  };

  class Recorder : private NonCopyable {

    bool enabled;   // enabled or not

    // files
    std::ofstream file;
    std::ofstream log;

    // structures
    RecorderInfo info;
    RecorderFrames frames;
    RecorderEvents events;
    RecorderPositions positions;

    // replayer
    Replayer replayer;

    public:

    Recorder();
    ~Recorder();
    void enable(void);
    void disable(void);
    bool isEnabled(void) { return enabled; };
    std::string start(std::string path, std::string name, std::string mapName);
    void stop(void);
    void clear(void);
    void addEvent(RecorderEventAdd _event);
    void addEvent(const RecorderEventDel _event);
    void addEvent(const RecorderEventParent _event);
    void addPosition(const RecorderPosition &_position);
    void write(void);


    // replayer
    Replayer &getReplayer(void) {
      return replayer;
    }
    std::string showFileInfo(std::string path, std::string name) {
      return replayer.getInfo(path + name);
    }
    std::string replayFile(std::string path, std::string name, double time) {
      stop();
      return replayer.replayFile(path + name, time);
    }

  };

}
}