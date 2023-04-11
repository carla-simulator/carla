#pragma once
#include "DataStructs.h"

namespace MeshReconstruction
{
  struct IntersectInfo
  {
    // 0 - 255
    int signConfig;

    // If it exists, vertex on edge i is stored at position i.
    // For edge numbering and location see numberings.png.
    std::array<Vec3, 12> edgeVertIndices;
  };

  class Cube
  {
    Vec3 pos[8];
    double sdf[8];

    Vec3 LerpVertex(double isoLevel, int i1, int i2) const;
    int SignConfig(double isoLevel) const;

  public:
    Cube(Rect3 const &space, Fun3s const &sdf);

    // Find the vertices where the surface intersects the cube.
    IntersectInfo Intersect(double isoLevel = 0) const;
  };

  namespace
  {
    // Cube has 8 vertices. Each vertex can have positive or negative sign.
    // 2^8 = 256 possible configurations mapped to intersected edges in each case.
    // The 12 edges are numbered as 1, 2, 4, ..., 2048 and are stored as a 12-bit bitstring for each configuration.
    const int signConfigToIntersectedEdges[256] = {
        0x0, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
        0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
        0x190, 0x99, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
        0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
        0x230, 0x339, 0x33, 0x13a, 0x636, 0x73f, 0x435, 0x53c,
        0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
        0x3a0, 0x2a9, 0x1a3, 0xaa, 0x7a6, 0x6af, 0x5a5, 0x4ac,
        0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
        0x460, 0x569, 0x663, 0x76a, 0x66, 0x16f, 0x265, 0x36c,
        0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
        0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff, 0x3f5, 0x2fc,
        0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
        0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55, 0x15c,
        0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
        0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc,
        0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
        0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
        0xcc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
        0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
        0x15c, 0x55, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
        0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
        0x2fc, 0x3f5, 0xff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
        0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
        0x36c, 0x265, 0x16f, 0x66, 0x76a, 0x663, 0x569, 0x460,
        0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
        0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa, 0x1a3, 0x2a9, 0x3a0,
        0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
        0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33, 0x339, 0x230,
        0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
        0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99, 0x190,
        0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
        0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0};

    struct Edge
    {
      int edgeFlag : 12; // flag: 1, 2, 4, ... 2048
      int vert0;         // 0-7
      int vert1;         // 0-7
    };

    const Edge edges[12] =
        {
            {1, 0, 1}, // edge 0
            {2, 1, 2}, // edge 1
            {4, 2, 3}, // ...
            {8, 3, 0},
            {16, 4, 5},
            {32, 5, 6},
            {64, 6, 7},
            {128, 7, 4},
            {256, 0, 4},
            {512, 1, 5},
            {1024, 2, 6},
            {2048, 3, 7} // edge 11
    };
  }

  Vec3 Cube::LerpVertex(double isoLevel, int i1, int i2) const
  {
    auto const Eps = 1e-5;
    auto const v1 = sdf[i1];
    auto const v2 = sdf[i2];
    auto const &p1 = pos[i1];
    auto const &p2 = pos[i2];

    if (abs(isoLevel - v1) < Eps)
      return p1;
    if (abs(isoLevel - v2) < Eps)
      return p2;
    if (abs(v1 - v2) < Eps)
      return p1;

    auto mu = (isoLevel - v1) / (v2 - v1);
    return p1 + (p2 - p1) * mu;
  }

  Cube::Cube(Rect3 const &space, Fun3s const &sdf)
  {
    auto mx = space.min.x;
    auto my = space.min.y;
    auto mz = space.min.z;

    auto sx = space.size.x;
    auto sy = space.size.y;
    auto sz = space.size.z;

    pos[0] = space.min;
    pos[1] = {mx + sx, my, mz};
    pos[2] = {mx + sx, my, mz + sz};
    pos[3] = {mx, my, mz + sz};
    pos[4] = {mx, my + sy, mz};
    pos[5] = {mx + sx, my + sy, mz};
    pos[6] = {mx + sx, my + sy, mz + sz};
    pos[7] = {mx, my + sy, mz + sz};

    for (auto i = 0; i < 8; ++i)
    {
      auto sd = sdf(pos[i]);
      if (sd == 0)
        sd += 1e-6;
      this->sdf[i] = sd;
    }
  }

  int Cube::SignConfig(double isoLevel) const
  {
    auto edgeIndex = 0;

    for (auto i = 0; i < 8; ++i)
    {
      if (sdf[i] < isoLevel)
      {
        edgeIndex |= 1 << i;
      }
    }

    return edgeIndex;
  }

  IntersectInfo Cube::Intersect(double iso) const
  {
    // idea:
    // from signs at 8 corners of cube a sign configuration (256 possible ones) is computed
    // this configuration can be used to index into a table that tells which of the 12 edges are intersected
    // find vertices adjacent to edges and interpolate cut vertex and store it in IntersectionInfo object

    IntersectInfo intersect;
    intersect.signConfig = SignConfig(iso);

    for (auto e = 0; e < 12; ++e)
    {
      if (signConfigToIntersectedEdges[intersect.signConfig] & edges[e].edgeFlag)
      {
        auto v0 = edges[e].vert0;
        auto v1 = edges[e].vert1;
        auto vert = LerpVertex(iso, v0, v1);
        intersect.edgeVertIndices[e] = vert;
      }
    }

    return intersect;
  }

}