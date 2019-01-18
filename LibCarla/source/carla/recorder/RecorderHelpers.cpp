// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "RecorderHelpers.h"

namespace carla {
namespace recorder {

// write binary data from Vector3D
void writeVector3D(std::ofstream &file, const carla::geom::Vector3D &obj) {
    //file.write(reinterpret_cast<const char*>(&obj.x), sizeof(float));
    //file.write(reinterpret_cast<const char*>(&obj.y), sizeof(float));
    //file.write(reinterpret_cast<const char*>(&obj.z), sizeof(float));
    writeValue<float>(file, obj.x);
    writeValue<float>(file, obj.y);
    writeValue<float>(file, obj.z);
}

// write binary data from Location
void writeLocation(std::ofstream &file, const carla::geom::Location &obj) {
    //file.write(reinterpret_cast<const char*>(&obj.x), sizeof(float));
    //file.write(reinterpret_cast<const char*>(&obj.y), sizeof(float));
    //file.write(reinterpret_cast<const char*>(&obj.z), sizeof(float));
    writeValue<float>(file, obj.x);
    writeValue<float>(file, obj.y);
    writeValue<float>(file, obj.z);
}

// write binary data from Rotation
void writeRotation(std::ofstream &file, const carla::geom::Rotation &obj) {
    writeValue<float>(file, obj.pitch);
    writeValue<float>(file, obj.yaw);
    writeValue<float>(file, obj.roll);
}

// write binary data from Transform
void writeTransform(std::ofstream &file, const carla::geom::Transform &obj){
    writeLocation(file, obj.location);
    writeRotation(file, obj.rotation);
}

// write binary data from string (length + text)
void writeString(std::ofstream &file, const std::string &obj) {
    short length = obj.size();

    writeValue<short>(file, length);    
    file.write(reinterpret_cast<const char*>(obj.c_str()), length);
}

}
}