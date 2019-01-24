// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <sstream>
#include <functional>
#include "carla/NonCopyable.h"
//#include "carla/recorder/Recorder.h"
#include "carla/recorder/RecorderFrames.h"
#include "carla/recorder/RecorderEvent.h"
#include "carla/recorder/RecorderPosition.h"
#include "carla/recorder/RecorderHelpers.h"

namespace carla {
namespace recorder {

//typedef std::function<bool (carla::geom::Transform, RecorderActorDescription)> RecorderCallbackEventAdd;
typedef std::function<bool ()> RecorderCallbackEventAdd;

#pragma pack(push, 1)
struct Header {
    char id;
    int size;
};
#pragma pack(pop)

class Replayer : private NonCopyable {
  
    bool enabled;   // enabled or not

    public:

    Replayer();
    ~Replayer();
    
    std::string getInfo(std::string filename);
    std::string replayFile(std::string filename, double time);
    //void start(void);
    //void stop(void);
    
    void enable(void);
    void disable(void);
    bool isEnabled(void) { return enabled; };

//void addEvent(const RecorderEventAdd &_event);
    //void addEvent(const RecorderEventDel &_event);
    //void addEvent(const RecorderEventParent &_event);
    //void addPosition(const RecorderPosition &_position);
    
    // callbacks
    void setCallbackEventAdd(RecorderCallbackEventAdd f);

    // tick for the replayer
    void tick(float time);

    private:
    Header header;
    RecorderFrame frame;
    //RecorderCallback2 *callbackEventAdd { nullptr };
    //boost::function<bool (carla::geom::Transform, RecorderActorDescription)> callbackEventAdd;
    RecorderCallbackEventAdd callbackEventAdd;

    bool readHeader(std::ifstream &file);
    void skipPacket(std::ifstream &file);
    void resetToTime(std::ifstream &file, double time, std::stringstream &info);
    void processEvents(std::ifstream &file, std::stringstream &info);
    void processPositions(std::ifstream &file, double per, std::stringstream &info);
    //void processCurrentFrame(std::ifstream &file, char flags = 0xff);
};

}
}