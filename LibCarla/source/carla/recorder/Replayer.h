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
#include "carla/recorder/RecorderInfo.h"
#include "carla/recorder/RecorderFrames.h"
#include "carla/recorder/RecorderEvent.h"
#include "carla/recorder/RecorderPosition.h"
#include "carla/recorder/RecorderState.h"
#include "carla/recorder/RecorderHelpers.h"

namespace carla {
namespace recorder {

// callback prototypes
typedef std::function<std::pair<int, uint32_t> (carla::geom::Transform, RecorderActorDescription, uint32_t uid)> RecorderCallbackEventAdd;
typedef std::function<bool (uint32_t uid)> RecorderCallbackEventDel;
typedef std::function<bool (uint32_t childId, uint32_t parentId)> RecorderCallbackEventParent;
typedef std::function<bool (RecorderPosition pos1, RecorderPosition pos2, double per)> RecorderCallbackPosition;
typedef std::function<bool (bool applyAutopilot)> RecorderCallbackFinish;
typedef std::function<bool (RecorderStateTrafficLight state)> RecorderCallbackStateTrafficLight;

#pragma pack(push, 1)
struct Header {
    char id;
    uint32_t size;
};
#pragma pack(pop)

class Replayer : private NonCopyable {

    public:

    Replayer();
    ~Replayer();

    std::string getInfo(std::string filename);
    std::string replayFile(std::string filename, double timeStart = 0.0f, double duration = 0.0f);
    //void start(void);
    void stop(bool keepActors = false);

    void enable(void);
    void disable(void);
    bool isEnabled(void) { return enabled; };

    // callbacks
    void setCallbackEventAdd(RecorderCallbackEventAdd f);
    void setCallbackEventDel(RecorderCallbackEventDel f);
    void setCallbackEventParent(RecorderCallbackEventParent f);
    void setCallbackEventPosition(RecorderCallbackPosition f);
    void setCallbackEventFinish(RecorderCallbackFinish f);
    void setCallbackStateTrafficLight(RecorderCallbackStateTrafficLight f);

    // tick for the replayer
    void tick(float time);

    private:
    bool enabled;
    // binary file reader
    std::ifstream file;
    Header header;
    RecorderInfo recInfo;
    RecorderFrame frame;
    // callbacks
    RecorderCallbackEventAdd callbackEventAdd;
    RecorderCallbackEventDel callbackEventDel;
    RecorderCallbackEventParent callbackEventParent;
    RecorderCallbackPosition callbackPosition;
    RecorderCallbackFinish callbackFinish;
    RecorderCallbackStateTrafficLight callbackStateTrafficLight;
    // positions (to be able to interpolate)
    std::vector<RecorderPosition> currPos;
    std::vector<RecorderPosition> prevPos;
    // mapping id
    std::unordered_map<uint32_t, uint32_t> mappedId;
    // times
    double currentTime;
    double timeToStop;
    double totalTime;

    // utils
    bool readHeader();
    void skipPacket();
    double getTotalTime(void);
    void rewind(void);

    // processing packets
    void processToTime(double time);
    void processEvents(void);
    void processPositions(void);
    void processStates(void);

    // positions
    void updatePositions(double per);
    void interpolatePosition(const RecorderPosition &start, const RecorderPosition &end, double per);

};

}
}