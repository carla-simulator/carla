// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Recorder.h"
#include "RecorderFrames.h"
#include "RecorderHelpers.h"

namespace carla {
namespace recorder {

void RecorderFrame::read(std::ifstream &file) {
    //readValue<unsigned long>(file, this.id);
    //readValue<double>(file, this.durationThis);
    //readValue<double>(file, this.elapsed);
    readValue<RecorderFrame>(file, *this);
}

void RecorderFrame::write(std::ofstream &file) {
    //writeValue<unsigned long>(file, frame.id);
    //writeValue<double>(file, frame.durationThis);
    //writeValue<double>(file, frame.elapsed);
    writeValue<RecorderFrame>(file, *this);
}

//---------------------------------------------

RecorderFrames::RecorderFrames(void) {
    reset();
}

void RecorderFrames::reset(void) {
    frame.id = 0;
    frame.durationThis = 0.0f;
    frame.elapsed = 0.0f;
    lastTime = std::chrono::high_resolution_clock::now();
}

void RecorderFrames::setFrame(void) {
    auto now = std::chrono::high_resolution_clock::now();

    if (frame.id == 0) {
        frame.elapsed = 0.0f;
        frame.durationThis = 0.0f;
    }
    else
    {
        frame.durationThis = std::chrono::duration<double>(now - lastTime).count();
        frame.elapsed += frame.durationThis;
    }

    lastTime = now;
    ++frame.id;
}

void RecorderFrames::write(std::ofstream &file, std::ofstream &log) {
    std::streampos pos, offset;
    double dummy = -1.0f;

    // write the packet id
    writeValue<char>(file, static_cast<char>(RecorderPacketId::Frame));

    // write the packet size
    int total = sizeof(RecorderFrame);
    writeValue<int>(file, total);

    // write frame record
    writeValue<unsigned long>(file, frame.id);
    offset = file.tellp();
    writeValue<double>(file, dummy);
    writeValue<double>(file, frame.elapsed);

    // we need to write this duration to previous frame
    if (offsetPreviousFrame > 0) {
        pos = file.tellp();
        file.seekp(offsetPreviousFrame, std::ios::beg);
        writeValue<double>(file, frame.durationThis);
        file.seekp(pos, std::ios::beg);
    }

    // save position for next actualization
    offsetPreviousFrame = offset;

    log << "frame " << frame.id << " elapsed." << frame.elapsed << ")\n";
}

}
}