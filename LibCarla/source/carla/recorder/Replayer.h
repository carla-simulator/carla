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

typedef std::function<bool (carla::geom::Transform, RecorderActorDescription, unsigned int uid)> RecorderCallbackEventAdd;
typedef std::function<bool (unsigned int uid)> RecorderCallbackEventDel;
typedef std::function<bool (unsigned int childId, unsigned int parentId)> RecorderCallbackEventParent;
typedef std::function<bool (RecorderPosition pos1, RecorderPosition pos2, double per)> RecorderCallbackPosition;

#pragma pack(push, 1)
struct Header {
    char id;
    int size;
};
#pragma pack(pop)

class Replayer : private NonCopyable {
  
    public:

    Replayer();
    ~Replayer();
    
    std::string getInfo(std::string filename);
    std::string replayFile(std::string filename, double time);
    //void start(void);
    void stop(void);
    
    void enable(void);
    void disable(void);
    bool isEnabled(void) { return enabled; };

    // callbacks
    void setCallbackEventAdd(RecorderCallbackEventAdd f);
    void setCallbackEventDel(RecorderCallbackEventDel f);
    void setCallbackEventParent(RecorderCallbackEventParent f);
    void setCallbackEventPosition(RecorderCallbackPosition f);

    // tick for the replayer
    void tick(float time);

    private:
    std::ifstream file;
    bool enabled;
    bool enablePlayback;
    Header header;
    RecorderFrame frame;
    RecorderCallbackEventAdd callbackEventAdd;
    RecorderCallbackEventDel callbackEventDel;
    RecorderCallbackEventParent callbackEventParent;
    RecorderCallbackPosition callbackPosition;
    double currentTime {0};
    std::vector<RecorderPosition> currPos;
    std::vector<RecorderPosition> prevPos;

    bool readHeader();
    void skipPacket();
    void rewind();
    void processToTime(double time);
    void processEvents();
    void processPositions(void);
    void updatePositions(double per);
    void interpolatePosition(const RecorderPosition &start, const RecorderPosition &end, double per);

};

}
}