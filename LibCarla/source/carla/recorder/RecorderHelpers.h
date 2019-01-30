// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include "carla/Buffer.h"
#include "carla/geom/Vector3D.h"
#include "carla/geom/Location.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Rotation.h"


namespace carla {
namespace recorder {

//---------
// recorder
//---------

// write binary data (using sizeof())
template<typename T>
void writeValue(std::ofstream &file, const T &obj) {
    file.write(reinterpret_cast<const char*>(&obj), sizeof(T));
}

// write binary data from Vector3D
void writeVector3D(std::ofstream &file, const carla::geom::Vector3D &obj);
// write binary data from Location
void writeLocation(std::ofstream &file, const carla::geom::Location &obj);
// write binary data from Transform
void writeTransform(std::ofstream &file, const carla::geom::Transform &obj);
// write binary data from Rotation
void writeRotation(std::ofstream &file, const carla::geom::Rotation &obj);
// write binary data from string (length + text)
void writeString(std::ofstream &file, const std::string &obj);
// write binary data from buffer (length + data)
void writeBuffer(std::ofstream &file, const Buffer &obj);

//---------
// replayer
//---------

// read binary data (using sizeof())
template<typename T>
void readValue(std::ifstream &file, T &obj) {
    file.read(reinterpret_cast<char *>(&obj), sizeof(T));
}

// read binary data from Vector3D
void readVector3D(std::ifstream &file, carla::geom::Vector3D &obj);
// read binary data from Location
void readLocation(std::ifstream &file, carla::geom::Location &obj);
// read binary data from Transform
void readTransform(std::ifstream &file, carla::geom::Transform &obj);
// read binary data from Rotation
void readRotation(std::ifstream &file, carla::geom::Rotation &obj);
// read binary data from string (length + text)
void readString(std::ifstream &file, std::string &obj);
// read binary data to buffer (length + data)
void readBuffer(std::ifstream &file, Buffer &obj);


}
}