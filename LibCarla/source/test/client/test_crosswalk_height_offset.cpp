// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/client/Map.h>
#include <carla/geom/Math.h>

#include <array>
#include <iomanip>
#include <locale>
#include <sstream>
#include <tuple>

namespace {

  struct Corner {
    double u;
    double v;
    double z;
  };

  const std::array<Corner, 5u> Corners{{
      {-2.0, -1.0, 0.0}, {-2.0, 1.0, 0.25}, {2.0, 1.0, 0.5},
      {2.0, -1.0, 0.75}, {-2.0, -1.0, 0.0}}};

  auto MakeMap(double offset, double elevation, double grade, double heading) {
    std::ostringstream xml;
    xml.imbue(std::locale::classic());
    xml << std::setprecision(17)
        << "<OpenDRIVE><header revMajor='1' revMinor='4'><geoReference>"
           "+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m"
           "</geoReference></header><road id='1' length='100' junction='-1' rule='RHT'><link/>"
           "<planView><geometry s='0' x='100' y='200' hdg='" << heading
        << "' length='100'><line/></geometry></planView><elevationProfile><elevation s='0' a='"
        << elevation << "' b='" << grade << "' c='0' d='0'/></elevationProfile>"
           "<lanes><laneSection s='0'><center><lane id='0' type='none'/></center><right>"
           "<lane id='-1' type='driving'><width sOffset='0' a='3.5' b='0' c='0' d='0'/></lane>"
           "</right></laneSection></lanes><objects><object id='1' name='crosswalk' type='crosswalk' "
           "s='30' t='0' zOffset='" << offset
        << "' hdg='0' pitch='0' roll='0' orientation='none' length='4' width='2'><outline>";
    for (const auto &corner : Corners) {
      xml << "<cornerLocal u='" << corner.u << "' v='" << corner.v << "' z='" << corner.z << "' height='0'/>";
    }
    xml << "</outline></object></objects></road></OpenDRIVE>";
    return carla::MakeShared<carla::client::Map>("crosswalk-height-offset", xml.str());
  }

  class CrosswalkHeightOffset : public testing::TestWithParam<std::tuple<double, double, double, double>> {};

  TEST_P(CrosswalkHeightOffset, includes_object_offset_and_corner_height_above_the_road) {
    const auto [offset, elevation, grade, heading] = GetParam();
    const auto map = MakeMap(offset, elevation, grade, heading);
    const auto zero_offset_map = MakeMap(0.0, elevation, grade, heading);
    const auto points = map->GetAllCrosswalkZones();
    const auto zero_offset_points = zero_offset_map->GetAllCrosswalkZones();
    ASSERT_EQ(points.size(), Corners.size());
    ASSERT_EQ(zero_offset_points.size(), points.size());
    for (size_t i = 0u; i < points.size(); ++i) {
      SCOPED_TRACE(i);
      // Retain CARLA's existing one-metre longitudinal padding and its
      // per-corner road-elevation sampling. This fix only adds object zOffset.
      const double padded_u = Corners[i].u + (Corners[i].u < 0.0 ? -1.0 : 1.0);
      const double expected_z = elevation + grade * (30.0 + padded_u) + Corners[i].z + offset;
      EXPECT_NEAR(points[i].z, expected_z, 0.0005);
      EXPECT_NEAR(points[i].z - zero_offset_points[i].z, offset, 0.0005);
      EXPECT_FLOAT_EQ(points[i].x, zero_offset_points[i].x);
      EXPECT_FLOAT_EQ(points[i].y, zero_offset_points[i].y);
    }
  }

  INSTANTIATE_TEST_SUITE_P(TranslatedAndGradedRoads, CrosswalkHeightOffset,
      testing::Combine(testing::Values(0.0, 0.05, 0.5, 2.0), testing::Values(0.0, 12.0),
          testing::Values(0.0, 0.1), testing::Values(0.0, carla::geom::Math::Pi<double>() / 2.0)));

} // namespace
