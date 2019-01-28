// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Replayer.h"
#include "Recorder.h"
#include "RecorderFrames.h"
#include "RecorderEvent.h"
#include "RecorderPosition.h"
#include "RecorderHelpers.h"
#include "carla/Logging.h"

#include <ctime>
#include <sstream>

namespace carla {
namespace recorder {

Replayer::Replayer(){
}

Replayer::~Replayer(){
    stop();
}

// callbacks
void Replayer::setCallbackEventAdd(RecorderCallbackEventAdd f) {
    callbackEventAdd = std::move(f);
}
void Replayer::setCallbackEventDel(RecorderCallbackEventDel f) {
    callbackEventDel = std::move(f);
}
void Replayer::setCallbackEventParent(RecorderCallbackEventParent f) {
    callbackEventParent = std::move(f);
}
void Replayer::setCallbackEventPosition(RecorderCallbackPosition f) {
    callbackPosition = std::move(f);
}

void Replayer::stop(void) {
    if (enabled) {
        enabled = false;
        processToTime(100000.0f);
        file.close();
    }
    log_warning("Replayer stopped");
}

bool Replayer::readHeader() {
    if (file.eof())
        return false;

    readValue<char>(file, header.id);
    readValue<int>(file, header.size);

    return true;
}

void Replayer::skipPacket() {
    file.seekg(header.size, std::ios::cur);
}

std::string Replayer::getInfo(std::string filename) {
    std::stringstream info;
    std::string s;
    
    // try to open
    file.open(filename, std::ios::binary);
    if (!file.is_open()) {
        info << "File " << filename << " not found on server\n";
        return info.str();
    }

    RecorderFrame frame;
    short i, total;
    RecorderEventAdd eventAdd;
    RecorderEventDel eventDel;
    RecorderEventParent eventParent;
    bool bShowFrame;

    // parse only frames
    while (file) {
        // get header
        if (!readHeader())
            break;
        
        // check for a frame packet
        switch (header.id) {
            case static_cast<char>(RecorderPacketId::Frame):
                frame.read(file);
                //info << "Frame " << frame.id << " at " << frame.elapsed << " seconds\n";
                break;
            case static_cast<char>(RecorderPacketId::Event):
                bShowFrame = true;
                readValue<short>(file, total);
                if (total > 0 && !bShowFrame) {
                    info << "Frame " << frame.id << " at " << frame.elapsed << " seconds\n";
                    bShowFrame = false;
                }
                for (i=0; i<total; ++i) {
                    eventAdd.read(file);
                    // convert buffer to string to show
                    s.resize(eventAdd.description.id.size());
                    std::copy(eventAdd.description.id.begin(), eventAdd.description.id.end(), s.begin());
                    info << "Create " << eventAdd.databaseId << ": " << s.data() << std::endl;
                }
                readValue<short>(file, total);
                if (total > 0 && !bShowFrame) {
                    info << "Frame " << frame.id << " at " << frame.elapsed << " seconds\n";
                    bShowFrame = false;
                }
                for (i=0; i<total; ++i) {
                    eventDel.read(file);
                    info << "Destroy " << eventDel.databaseId << "\n";
                }
                readValue<short>(file, total);
                if (total > 0 && !bShowFrame) {
                    info << "Frame " << frame.id << " at " << frame.elapsed << " seconds\n";
                    bShowFrame = false;
                }
                for (i=0; i<total; ++i) {
                    eventParent.read(file);
                    info << "Parenting " << eventParent.databaseId << " with " << eventDel.databaseId << " (parent)\n";
                }
                break;
            case static_cast<char>(RecorderPacketId::Position):
                //info << "Positions\n";
                skipPacket();
                break;
            default:
                // skip packet
                info << "Unknown packet id: " << header.id << " at offset " << file.tellg() << std::endl;
                skipPacket();
                break;
        }
    }

    info << "Frames: " << frame.id << "\n";
    info << "Duration: " << frame.elapsed << " seconds\n";

    file.close();

    return info.str();
}

void Replayer::rewind() {
    currentTime = 0.0f;
    file.seekg(0, std::ios::beg);

    // mark as header as invalid to force reload a new one next time
    frame.elapsed = -1.0f;
    frame.durationThis = 0.0f;

    log_warning("Replayer rewind");

}

std::string Replayer::replayFile(std::string filename, double time) {
    std::stringstream info;
    std::string s;
    
    // check toi stop if we are replaying another
    if (enabled)
        stop();

    info << "Replaying file: " << filename << std::endl;

    // try to open
    file.open(filename, std::ios::binary);
    if (!file.is_open()) {
        info << "File " << filename << " not found on server\n";
        return info.str();
    }

    // from start
    rewind();

    // process all events until the time
    processToTime(time);

    // mark as enabled
    enabled = true;
    enablePlayback = true;

    return info.str();
}

void Replayer::processToTime(double time) {
    double per = 0.0f;
    double newTime = currentTime + time;
    bool bEnd = false;

    // check if we are in the right frame
    if (newTime >= frame.elapsed && newTime < frame.elapsed + frame.durationThis)
    {    
        per = (newTime - frame.elapsed) / frame.durationThis;
        bEnd = true;
    }

    // process all frames until time we want or end
    while (!file.eof() && !bEnd) {
        
        // get header
        readHeader();
        // check it is a frame packet
        if (header.id != static_cast<char>(RecorderPacketId::Frame)) {
            stop();
            break;
        }
        // read current frame
        frame.read(file);

        // check if target time is in this frame
        if (frame.elapsed + frame.durationThis < newTime)
            per = 0.0f;
        else {
            per = (newTime - frame.elapsed) / frame.durationThis;
            bEnd = true;
        }

        //info << "Frame: " << frame.id << " (" << frame.durationThis << " / " << frame.elapsed << ") per: " << per << std::endl;

        // get header
        readHeader();
        // check it is an events packet
        if (header.id != static_cast<char>(RecorderPacketId::Event)) {
            stop();
            break;
        }
        processEvents();

        // get header
        readHeader();
        // check it is a positions packet
        if (header.id != static_cast<char>(RecorderPacketId::Position)) {
            stop();
            break;
        }
        if (bEnd)
            processPositions();
        else
            skipPacket();

        // todo: status
        //processStatus(file);

        //log_warning("Replayer new frame");
    }

    // update all positions
    updatePositions(per);

    // save current time
    currentTime = newTime;
}

void Replayer::processEvents(void) {
    short i, total;
    RecorderEventAdd eventAdd;
    RecorderEventDel eventDel;
    RecorderEventParent eventParent;
    std::stringstream info;

    // create events
    readValue<short>(file, total);
    std::string s, s2;
    for (i=0; i<total; ++i) {
        eventAdd.read(file);

        s.resize(eventAdd.description.id.size());
        std::copy(eventAdd.description.id.begin(), eventAdd.description.id.end(), s.begin());
        info << "Create " << eventAdd.databaseId << " (" << eventAdd.description.uid << ") " << s.data() << std::endl;
        for (const auto &att : eventAdd.description.attributes) {
            s.resize(att.id.size());
            std::copy(att.id.begin(), att.id.end(), s.begin());
            s2.resize(att.value.size());
            std::copy(att.value.begin(), att.value.end(), s2.begin());
            info << "  " << s.data() << " = " << s2.data() << std::endl;
        }
        log_warning(info.str());

        // callback
        if (callbackEventAdd) {
            log_warning("calling callback add");
            callbackEventAdd(eventAdd.transform, std::move(eventAdd.description), eventAdd.databaseId);
        }
        else
            log_warning("callback add is not defined");
    }
    
    // destroy events
    readValue<short>(file, total);
    for (i=0; i<total; ++i) {
        eventDel.read(file);
        info << "Destroy " << eventDel.databaseId << "\n";
        log_warning(info.str());
        // callback
        if (callbackEventDel) {
            log_warning("calling callback del");
            callbackEventDel(eventDel.databaseId);
        }
        else
            log_warning("callback del is not defined");
    }

    // parenting events
    readValue<short>(file, total);
    for (i=0; i<total; ++i) {
        eventParent.read(file);
        info << "Parenting " << eventParent.databaseId << " with " << eventDel.databaseId << " (parent)\n";
        log_warning(info.str());
        // callback
        if (callbackEventParent) {
            log_warning("calling callback parent");
            callbackEventParent(eventParent.databaseId, eventParent.databaseIdParent);
        }
        else
            log_warning("callback parent is not defined");
    }
}

void Replayer::processPositions(void) {
    short i, total;

    // save current as previous
    prevPos = std::move(currPos);

    // read all positions
    readValue<short>(file, total);
    currPos.clear();
    currPos.reserve(total);
    for (i=0; i<total; ++i) {
        RecorderPosition pos;
        pos.read(file);
        currPos.push_back(std::move(pos));
    }
}

void Replayer::updatePositions(double per) {
    unsigned int i;
    std::unordered_map<int, int> mapId;

    // map the Id's from both vectors
    for (i=0; i<prevPos.size(); ++i)
        mapId[prevPos[i].databaseId] = i;

    // go through each actor and update
    for (i=0; i<currPos.size(); ++i) {
        // check if exist a previous position
        auto result = mapId.find(currPos[i].databaseId);
        if (result != mapId.end()) {
            // interpolate
            interpolatePosition(prevPos[result->second], currPos[i], per);
        }
        else {
            // assign last position (we don't have previous one)
            interpolatePosition(currPos[i], currPos[i], 0);
            log_warning("Interpolation not possible, only one position");
        }
    }
}

// interpolate a position (transform, velocity...)
void Replayer::interpolatePosition(const RecorderPosition &prevPos, const RecorderPosition &currPos, double per) {
    // call the callback
    callbackPosition(prevPos, currPos, per);
}

// tick for the replayer
void Replayer::tick(float delta) {
    // check if there are events to process
    if (enabled)
        processToTime(delta);

    //log_warning("Replayer tick");
 }

}
}