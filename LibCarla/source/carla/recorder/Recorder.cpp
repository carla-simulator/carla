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

std::string Recorder::start(std::string path, std::string map) {
    
    // reset
    stop();

    // prepare the target file
    if (map == "")
        map = "recorder";

    // get current time
    std::time_t timet = std::time(0);
    std::tm *now = std::localtime(&timet);

    std::stringstream name;
    name << path;
    name << (1900 + now->tm_year);
    name << std::setw(2) << std::setfill('0') << (now->tm_mon + 1);
    name << std::setw(2) << std::setfill('0') << (now->tm_mday) << "_";
    name << std::setw(2) << std::setfill('0') << (now->tm_hour);
    name << std::setw(2) << std::setfill('0') << (now->tm_min);
    name << std::setw(2) << std::setfill('0') << (now->tm_sec);
    name << "_" << map << ".rec";
    
    // files
    file.open(name.str(), std::ios::binary | std::ios::trunc | std::ios::out);
    std::string filename(name.str());

    // log
    name << ".log";
    log.open(name.str());

    enabled = true;

    return filename;
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
    
    // prepare the buffer with the content
    frames.write(file, log);
    events.write(file, log);
    positions.write(file, log);

    clear();
}

void Recorder::addPosition(const RecorderPosition &position) {
    positions.addPosition(position);
}

void Recorder::addEvent(const RecorderEvent &event) {
    events.addEvent(event);
}

}
}