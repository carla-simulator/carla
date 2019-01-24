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

// write binary data from buffer (length + data)
void writeBuffer(std::ofstream &file, const Buffer &obj) {
    short length = obj.size();
    writeValue<short>(file, length);    
    file.write(reinterpret_cast<const char*>(obj.data()), length);
}

// read binary data from Vector3D
void readVector3D(std::ifstream &file, carla::geom::Vector3D &obj) {
    readValue<float>(file, obj.x);
    readValue<float>(file, obj.y);
    readValue<float>(file, obj.z);
}

// read binary data from Location
void readLocation(std::ifstream &file, carla::geom::Location &obj) {
    readValue<float>(file, obj.x);
    readValue<float>(file, obj.y);
    readValue<float>(file, obj.z);
}

// read binary data from Rotation
void readRotation(std::ifstream &file, carla::geom::Rotation &obj) {
    readValue<float>(file, obj.pitch);
    readValue<float>(file, obj.yaw);
    readValue<float>(file, obj.roll);
}

// read binary data from Transform
void readTransform(std::ifstream &file, carla::geom::Transform &obj){
    readLocation(file, obj.location);
    readRotation(file, obj.rotation);
}

// read binary data from string (length + text)
void readString(std::ifstream &file, std::string &obj) {
    short length;
    readValue<short>(file, length);
    obj.resize(length);
    file.read(&obj[0], length);
}

// read binary data to buffer (length + data)
void readBuffer(std::ifstream &file, Buffer &obj) {
    short length;
    readValue<short>(file, length);
    obj.reset(static_cast<unsigned int>(length));
    file.read(reinterpret_cast<char *>(obj.data()), length);
}

}
}