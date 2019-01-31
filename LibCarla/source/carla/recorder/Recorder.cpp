// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Recorder.h"

#include <ctime>
#include <sstream>

namespace carla {
namespace recorder {


Recorder::Recorder(){
}

Recorder::~Recorder(){
    // close files
    if (file) file.close();
    if (log) log.close();
}

std::string Recorder::start(std::string path, std::string name, std::string mapName) {

    // reset
    stop();

/*
    // get current time
    std::time_t timet = std::time(0);
    std::tm *now = std::localtime(&timet);

    std::stringstream filename;
    filename << path;
    filename << (1900 + now->tm_year);
    filename << std::setw(2) << std::setfill('0') << (now->tm_mon + 1);
    filename << std::setw(2) << std::setfill('0') << (now->tm_mday) << "_";
    filename << std::setw(2) << std::setfill('0') << (now->tm_hour);
    filename << std::setw(2) << std::setfill('0') << (now->tm_min);
    filename << std::setw(2) << std::setfill('0') << (now->tm_sec);
    filename << "_" << map << ".rec";
*/
    std::stringstream filename;
    filename << path << name;

    // files
    file.open(filename.str(), std::ios::binary | std::ios::trunc | std::ios::out);
    if (!file.is_open()) {
        log_error("Recorder file couldn't be created");
        return "";
    }

    // log
    log.open(filename.str() + ".log");

    std::string magic("CARLA_RECORDER");
    // save info
    info.version = 1;
    info.magic.copy_from(reinterpret_cast<const unsigned char *>(magic.c_str()), magic.size());
    info.date = std::time(0);
    info.mapfile.copy_from(reinterpret_cast<const unsigned char *>(mapName.c_str()), mapName.size());

    // write general info
    info.write(file);

    frames.reset();
    enabled = true;

    return filename.str();
}

void Recorder::stop() {
    clear();
    enabled = false;
    if (file) file.close();
    if (log) log.close();
}

void Recorder::clear(void) {
    events.clear();
    positions.clear();
    log << "Clear\n";
}

void Recorder::write(void) {
    // update this frame data
    frames.setFrame();

    frames.write(file, log);
    events.write(file, log);
    positions.write(file, log);

    clear();
}

void Recorder::addPosition(const RecorderPosition &position) {
    if (enabled)
        positions.addPosition(position);
}

void Recorder::addEvent(RecorderEventAdd event) {
    if (enabled)
        events.addEvent(std::move(event));
}
void Recorder::addEvent(const RecorderEventDel event) {
    if (enabled)
        events.addEvent(std::move(event));
}
void Recorder::addEvent(const RecorderEventParent event) {
    if (enabled)
        events.addEvent(std::move(event));
}

}
}