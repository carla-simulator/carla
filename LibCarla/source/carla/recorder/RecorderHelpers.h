// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include "carla/geom/Vector3D.h"
#include "carla/geom/Location.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Rotation.h"


namespace carla {
namespace recorder {

// write binary data from int
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

}
}