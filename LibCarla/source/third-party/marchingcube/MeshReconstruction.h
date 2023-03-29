#pragma once
#include "DataStructs.h"
#include "Cube.h"
#include "Triangulation.h"

namespace MeshReconstruction
{
  /// Reconstructs a triangle mesh from a given signed distance function using <a href="https://en.wikipedia.org/wiki/Marching_cubes">Marching Cubes</a>.
  /// @param sdf The <a href="http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm">Signed Distance Function</a>.
  /// @param domain Domain of reconstruction.
  /// @returns The reconstructed mesh.
  Mesh MarchCube(
      Fun3s const &sdf,
      Rect3 const &domain);

  /// Reconstructs a triangle mesh from a given signed distance function using <a href="https://en.wikipedia.org/wiki/Marching_cubes">Marching Cubes</a>.
  /// @param sdf The <a href="http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm">Signed Distance Function</a>.
  /// @param domain Domain of reconstruction.
  /// @param cubeSize Size of marching cubes. Smaller cubes yields meshes of higher resolution.
  /// @param isoLevel Level set of the SDF for which triangulation should be done. Changing this value moves the reconstructed surface.
  /// @param sdfGrad Gradient of the SDF which yields the vertex normals of the reconstructed mesh. If none is provided a numerical approximation is used.
  /// @returns The reconstructed mesh.
  Mesh MarchCube(
      Fun3s const &sdf,
      Rect3 const &domain,
      Vec3 const &cubeSize,
      double isoLevel = 0,
      Fun3v sdfGrad = nullptr);
}

using namespace MeshReconstruction;
using namespace std;

// Adapted from here: http://paulbourke.net/geometry/polygonise/

namespace
{
  Vec3 NumGrad(Fun3s const &f, Vec3 const &p)
  {
    auto const Eps = 1e-6;
    Vec3 epsX{Eps, 0, 0}, epsY{0, Eps, 0}, epsZ{0, 0, Eps};
    auto gx = (f(p + epsX) - f(p - epsX)) / 2;
    auto gy = (f(p + epsY) - f(p - epsY)) / 2;
    auto gz = (f(p + epsZ) - f(p - epsZ)) / 2;
    return {gx, gy, gz};
  }
}

Mesh MeshReconstruction::MarchCube(Fun3s const &sdf, Rect3 const &domain)
{
  auto const NumCubes = 50;
  auto cubeSize = domain.size * (1.0 / NumCubes);

  return MarchCube(sdf, domain, cubeSize);
}

Mesh MeshReconstruction::MarchCube(
    Fun3s const &sdf,
    Rect3 const &domain,
    Vec3 const &cubeSize,
    double isoLevel,
    Fun3v sdfGrad)
{
  // Default value.
  sdfGrad = sdfGrad == nullptr
                ? [&sdf](Vec3 const &p)
  { return NumGrad(sdf, p); }
                : sdfGrad;

  auto const NumX = static_cast<int>(ceil(domain.size.x / cubeSize.x));
  auto const NumY = static_cast<int>(ceil(domain.size.y / cubeSize.y));
  auto const NumZ = static_cast<int>(ceil(domain.size.z / cubeSize.z));

  auto const HalfCubeDiag = cubeSize.Norm() / 2.0;
  auto const HalfCubeSize = cubeSize * 0.5;

  Mesh mesh;

  for (auto ix = 0; ix < NumX; ++ix)
  {
    auto x = domain.min.x + ix * cubeSize.x;

    for (auto iy = 0; iy < NumY; ++iy)
    {
      auto y = domain.min.y + iy * cubeSize.y;

      for (auto iz = 0; iz < NumZ; ++iz)
      {
        auto z = domain.min.z + iz * cubeSize.z;
        Vec3 min{x, y, z};

        // Process only if cube lies within narrow band around surface.
        auto cubeCenter = min + HalfCubeSize;
        auto dist = abs(sdf(cubeCenter) - isoLevel);
        if (dist > HalfCubeDiag)
          continue;

        Cube cube({min, cubeSize}, sdf);
        auto intersect = cube.Intersect(isoLevel);
        Triangulate(intersect, sdfGrad, mesh);
      }
    }
  }

  return mesh;
}
