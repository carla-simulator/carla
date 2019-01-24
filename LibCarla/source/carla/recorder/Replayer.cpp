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
    // close files
    //if (file) file.close();
}

// callbacks
void Replayer::setCallbackEventAdd(RecorderCallbackEventAdd f) {
    callbackEventAdd = std::move(f);

    //std::ofstream file("logger.log", std::ios::app);
    if (callbackEventAdd)
        log_warning("true");
        //file << "True" << endl;
    else
        log_warning("false");
        //file << "False" << endl;
    //file << static_cast<long>(f) << " / " << static_cast<long>(callbackEventAdd.target) << std::endl;
    //file.close();
}

bool Replayer::readHeader(std::ifstream &file) {
    if (file.eof())
        return false;

    readValue<char>(file, header.id);
    readValue<int>(file, header.size);

    return true;
}

void Replayer::skipPacket(std::ifstream &file) {
    file.seekg(header.size, std::ios::cur);
}

std::string Replayer::getInfo(std::string filename) {
    std::ifstream f;
    std::stringstream info;
    std::string s;
    
    // try to open
    f.open(filename, std::ios::binary);
    if (!f.is_open()) {
        info << "File " << filename << " not found on server\n";
        return info.str();
    }

    RecorderFrame frame;
    short i, total;
    RecorderEventAdd eventAdd;
    RecorderEventDel eventDel;
    RecorderEventParent eventParent;

    // parse only frames
    while (f) {
        // get header
        if (!readHeader(f))
            break;
        
        // check for a frame packet
        switch (header.id) {
            case static_cast<char>(RecorderPacketId::Frame):
                frame.read(f);
                info << "Frame " << frame.id << " at " << frame.elapsed << " seconds\n";
                break;
            case static_cast<char>(RecorderPacketId::Event):
                readValue<short>(f, total);
                for (i=0; i<total; ++i) {
                    eventAdd.read(f);
                    // convert buffer to string to show
                    s.resize(eventAdd.description.id.size());
                    std::copy(eventAdd.description.id.begin(), eventAdd.description.id.end(), s.begin());
                    info << "Create " << eventAdd.databaseId << ": " << s.data() << std::endl;
                }
                readValue<short>(f, total);
                for (i=0; i<total; ++i) {
                    eventDel.read(f);
                    info << "Destroy " << eventDel.databaseId << "\n";
                }
                readValue<short>(f, total);
                for (i=0; i<total; ++i) {
                    eventParent.read(f);
                    info << "Parenting " << eventParent.databaseId << " with " << eventDel.databaseId << " (parent)\n";
                }
                break;
            case static_cast<char>(RecorderPacketId::Position):
                //info << "Positions\n";
                skipPacket(f);
                break;
            default:
                // skip packet
                info << "Unknown packet id: " << header.id << " at offset " << f.tellg() << std::endl;
                skipPacket(f);
                break;
        }
    }

    info << "Frames: " << frame.id << "\n";
    info << "Duration: " << frame.elapsed << " seconds\n";

    f.close();

    return info.str();
}

std::string Replayer::replayFile(std::string filename, double time) {
    std::ifstream f;
    std::stringstream info;
    std::string s;
    
    info << "Replaying file: " << filename << std::endl;

    // try to open
    f.open(filename, std::ios::binary);
    if (!f.is_open()) {
        info << "File " << filename << " not found on server\n";
        return info.str();
    }

    // mark as enabled
    enabled = true;

    // prepare scene until that time
    resetToTime(f, time, info);

    return info.str();
}

void Replayer::resetToTime(std::ifstream &file, double time, std::stringstream &info) {
    double per;

    // force start of file
    file.seekg(0, std::ios::beg);

    info << "Starting until time " << time << std::endl;

    // process all frames until time we want or end
    bool bReady = false;
    while (!file.eof() && !bReady) {
        
        // get header
        readHeader(file);
        // check it is a frame packet
        if (header.id != static_cast<char>(RecorderPacketId::Frame))
            break;
        // read current frame
        frame.read(file);

        // check if target time is in this frame
        if (time > frame.elapsed + frame.durationThis)
            per = 0.0f;
        else
            per = (time - frame.elapsed) / frame.durationThis;

        info << "Frame: " << frame.id << " (" << frame.durationThis << " / " << frame.elapsed << ") per: " << per << std::endl;

        // get header
        readHeader(file);
        // check it is an events packet
        if (header.id != static_cast<char>(RecorderPacketId::Event))
            break;
        processEvents(file, info);

        // get header
        readHeader(file);
        // check it is a positions packet
        if (header.id != static_cast<char>(RecorderPacketId::Position))
            break;
        skipPacket(file);
/*
        if (per > 0) {
            processPositions(file, per, info);
            bReady = true;
        }
        else
            skipPacket(file);
*/
        // todo: status
        //processStatus(file);
    }
}

void Replayer::processEvents(std::ifstream &file, std::stringstream &info) {
    short i, total;
    RecorderEventAdd eventAdd;
    RecorderEventDel eventDel;
    RecorderEventParent eventParent;

    // create events
    readValue<short>(file, total);
    std::string s;
    for (i=0; i<total; ++i) {
        eventAdd.read(file);
        
        // convert buffer to string to show
        s.resize(eventAdd.description.id.size());
        std::copy(eventAdd.description.id.begin(), eventAdd.description.id.end(), s.begin());
        info << "Create " << eventAdd.databaseId << ": " << s.data() << std::endl;

        // call
        //info << "Creating actor: " << eventAdd.description.databaseId << std::endl;
        if (callbackEventAdd) {
            //info << "Calling callback: " << callbackEventAdd(eventAdd.transform, std::move(eventAdd.description)) << std::endl;
            info << "Calling callback: " << callbackEventAdd() << std::endl;
        }
    }
    
    // destroy events
    readValue<short>(file, total);
    for (i=0; i<total; ++i) {
        eventDel.read(file);
        info << "Destroy " << eventDel.databaseId << "\n";
    }

    // parenting events
    readValue<short>(file, total);
    for (i=0; i<total; ++i) {
        eventParent.read(file);
        info << "Parenting " << eventParent.databaseId << " with " << eventDel.databaseId << " (parent)\n";
    }
}

void Replayer::processPositions(std::ifstream &file, double per, std::stringstream &info) {
    if (file.eof())
        per = 1;
    info << "Skip positions\n";
}

// tick for the replayer
void Replayer::tick(float time) {
    // check if there are events to process
    float f = time;
    f += 5.0;
}

}
}