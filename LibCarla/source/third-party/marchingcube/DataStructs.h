#pragma once
#include <vector>
#include <array>
#include <functional>

namespace MeshReconstruction
{
  struct Vec3
  {
    double x, y, z;

    Vec3 operator+(Vec3 const &o) const
    {
      return {x + o.x, y + o.y, z + o.z};
    }

    Vec3 operator-(Vec3 const &o) const
    {
      return {x - o.x, y - o.y, z - o.z};
    }

    Vec3 operator*(double c) const
    {
      return {c * x, c * y, c * z};
    }

    double Norm() const
    {
      return sqrt(x * x + y * y + z * z);
    }

    Vec3 Normalized() const
    {
      auto n = Norm();
      return {x / n, y / n, z / n};
    }
  };

  struct Rect3
  {
    Vec3 min;
    Vec3 size;
  };

  using Triangle = std::array<int, 3>;

  struct Mesh
  {
    std::vector<Vec3> vertices;
    std::vector<Triangle> triangles;
    std::vector<Vec3> vertexNormals;
  };

  using Fun3s = std::function<double(Vec3 const &)>;
  using Fun3v = std::function<Vec3(Vec3 const &)>;
}