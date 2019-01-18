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

RecorderFrames::RecorderFrames(void) {
    frame.id = 0;
    frame.delta = 0.0f;
    frame.elapsed = 0.0f;
    lastTime = std::chrono::high_resolution_clock::now();
}

void RecorderFrames::setFrame(void) {
    auto now = std::chrono::high_resolution_clock::now();
    
    if (frame.id == 0) {
        frame.elapsed = 0.0f;
        frame.delta = 0.0f;
    }
    else
    {
        frame.delta = std::chrono::duration<double>(now - lastTime).count();
        frame.elapsed += frame.delta;
    }

    lastTime = now;
    ++frame.id;
}

void RecorderFrames::write(std::ofstream &file, std::ofstream &log) {

    // write the packet id
    writeValue<char>(file, static_cast<char>(RecorderPacketId::Frame));
    
    // write the packet size
    int total = sizeof(RecorderFrame);
    writeValue<int>(file, total);

    // write frame record
    writeValue<RecorderFrame>(file, frame);
    
    log << "frame " << frame.id << " (delta." << frame.delta << " elapsed." << frame.elapsed << ")\n";
}

}
}