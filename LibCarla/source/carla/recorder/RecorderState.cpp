// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Recorder.h"
#include "RecorderState.h"
#include "RecorderHelpers.h"

namespace carla {
namespace recorder {

void RecorderStateTrafficLight::write(std::ofstream &file) {
    writeValue<uint32_t>(file, this->databaseId);
    writeValue<bool>(file, this->isFrozen);
    writeValue<float>(file, this->elapsedTime);
    writeValue<char>(file, this->state);
}

void RecorderStateTrafficLight::read(std::ifstream &file) {
    readValue<uint32_t>(file, this->databaseId);
    readValue<bool>(file, this->isFrozen);
    readValue<float>(file, this->elapsedTime);
    readValue<char>(file, this->state);
}

//---------------------------------------------

void RecorderStates::clear(void) {
    statesTrafficLights.clear();
}

void RecorderStates::addState(RecorderStateTrafficLight state) {
    statesTrafficLights.push_back(std::move(state));
}

void RecorderStates::writeStatesTrafficLight(std::ofstream &file) {
    // write total records
    short total = statesTrafficLights.size();
    writeValue<short>(file, total);

    for (unsigned long i=0; i<statesTrafficLights.size(); ++i) {
        statesTrafficLights[i].write(file);
    }
}

void RecorderStates::write(std::ofstream &file, std::ofstream &log) {

    // write the packet id
    writeValue<char>(file, static_cast<char>(RecorderPacketId::State));

    std::streampos posStart = file.tellp();

    // write a dummy packet size
    int total = 0;
    writeValue<int>(file, total);

    // write events
    writeStatesTrafficLight(file);

    // write the real packet size
    std::streampos posEnd = file.tellp();
    total = posEnd - posStart - sizeof(int);
    file.seekp(posStart, std::ios::beg);
    writeValue<int>(file, total);
    file.seekp(posEnd, std::ios::beg);

    log << "write states (" << statesTrafficLights.size() << ")\n";
}

}
}