#include <carla/Version.h>
#include <carla/geom/Vector3D.h>
#include <carla/geom/Transform.h>
#include <iostream>

int main() {
    std::cout << "=== CARLA libcarla Test ===" << std::endl;
    std::cout << "CARLA Version: " << carla::version() << std::endl;

    // Vector3D test
    carla::geom::Vector3D vec(1.0f, 2.0f, 3.0f);
    std::cout << "Vector3D: ("
              << vec.x << ", " << vec.y << ", " << vec.z << ")"
              << std::endl;

    // Transform test
    carla::geom::Location loc(10.0f, 20.0f, 30.0f);
    carla::geom::Rotation rot(0.0f, 90.0f, 0.0f);
    carla::geom::Transform transform(loc, rot);

    std::cout << "Transform location: ("
              << transform.location.x << ", "
              << transform.location.y << ", "
              << transform.location.z << ")"
              << std::endl;

    std::cout << "Test passed!" << std::endl;
    return 0;
}
