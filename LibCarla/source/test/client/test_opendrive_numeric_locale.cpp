// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/client/Map.h"
#include "carla/client/Waypoint.h"
#include "carla/geom/GeoLocation.h"
#include <third-party/pugixml/pugixml.hpp>

#include <atomic>
#include <clocale>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <locale.h>
#include <thread>

namespace {

  class ScopedNumericLocale {
  public:
    bool Enable(const char *name) {
#ifdef _WIN32
      if (!_active) {
        _previous_mode = _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
        if (_previous_mode == -1) {
          return false;
        }
        _previous = std::setlocale(LC_NUMERIC, nullptr);
        _active = true;
      }
      return std::setlocale(LC_NUMERIC, name) != nullptr;
#else
      const auto locale = newlocale(LC_NUMERIC_MASK, name, nullptr);
      if (locale == nullptr) {
        return false;
      }
      const auto previous = uselocale(locale);
      if (previous == nullptr) {
        freelocale(locale);
        return false;
      }
      if (!_active) {
        _previous = previous;
        _active = true;
      } else {
        freelocale(_locale);
      }
      _locale = locale;
      return true;
#endif
    }

    bool EnableCommaDecimal() {
      for (const auto *name : {"German_Germany.1252", "de_DE.UTF-8", "de_DE.utf8", "de_DE"}) {
        if (Enable(name)) {
          return true;
        }
      }
      return false;
    }

    ~ScopedNumericLocale() {
      if (_active) {
#ifdef _WIN32
        std::setlocale(LC_NUMERIC, _previous.c_str());
        _configthreadlocale(_previous_mode);
#else
        uselocale(_previous);
        freelocale(_locale);
#endif
      }
    }

  private:
#ifdef _WIN32
    std::string _previous;
    int _previous_mode = 0;
#else
    locale_t _previous = nullptr;
    locale_t _locale = nullptr;
#endif
    bool _active = false;
  };

  carla::SharedPtr<carla::client::Map> LoadMap() {
    return carla::MakeShared<carla::client::Map>("numeric-locale", R"(
<OpenDRIVE><header revMajor="1" revMinor="4"><geoReference>
+proj=tmerc +lat_0=49.5 +lon_0=9.5 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m
</geoReference></header><road id="1" length="20" junction="-1" rule="RHT">
<planView><geometry s="0" x="0" y="0" hdg="0" length="20"><line/></geometry></planView>
<elevationProfile><elevation s="0" a="1.75" b="0" c="0" d="0"/></elevationProfile>
<lanes><laneSection s="0"><center><lane id="0" type="none"/></center><right>
<lane id="-1" type="driving"><width sOffset="0" a="3.5" b="0" c="0" d="0"/>
</lane></right></laneSection></lanes></road></OpenDRIVE>)");
  }

  class OpenDriveNumericLocale : public testing::Test {
  protected:
    void SetUp() override {
      if (!_locale.EnableCommaDecimal()) {
        GTEST_SKIP() << "An installed comma-decimal locale is required";
      }
      ASSERT_STREQ(std::localeconv()->decimal_point, ",");
    }

    ScopedNumericLocale _locale;
  };

} // namespace

TEST_F(OpenDriveNumericLocale, lane_width_uses_xml_decimal_point) {
  const auto map = LoadMap();
  const auto waypoint = map->GetWaypointXODR(1u, -1, 5.0f);
  ASSERT_NE(waypoint, nullptr);
  EXPECT_DOUBLE_EQ(waypoint->GetLaneWidth(), 3.5);
}

TEST_F(OpenDriveNumericLocale, elevation_uses_xml_decimal_point) {
  const auto map = LoadMap();
  const auto waypoint = map->GetWaypointXODR(1u, -1, 5.0f);
  ASSERT_NE(waypoint, nullptr);
  EXPECT_FLOAT_EQ(waypoint->GetTransform().location.z, 1.75f);
}

TEST_F(OpenDriveNumericLocale, projection_origin_uses_c_decimal_point) {
  const auto reference = LoadMap()->GetGeoReference();
  EXPECT_DOUBLE_EQ(reference.latitude, 49.5);
  EXPECT_DOUBLE_EQ(reference.longitude, 9.5);
}

TEST_F(OpenDriveNumericLocale, xml_double_and_float_attributes) {
  pugi::xml_document xml;
  ASSERT_TRUE(xml.load_string("<root a='3.5' b='1.75'/>"));
  EXPECT_DOUBLE_EQ(xml.child("root").attribute("a").as_double(), 3.5);
  EXPECT_FLOAT_EQ(xml.child("root").attribute("b").as_float(), 1.75f);
}

TEST_F(OpenDriveNumericLocale, xml_text_and_xpath_numbers) {
  pugi::xml_document xml;
  ASSERT_TRUE(xml.load_string("<root>3.5</root>"));
  EXPECT_DOUBLE_EQ(xml.child("root").text().as_double(), 3.5);
  EXPECT_FLOAT_EQ(xml.child("root").text().as_float(), 3.5f);
  EXPECT_DOUBLE_EQ(pugi::xpath_query("number('3.5')").evaluate_number(xml.child("root")), 3.5);
}

TEST_F(OpenDriveNumericLocale, c_locale_map_control) {
  ASSERT_TRUE(_locale.Enable("C"));
  const auto map = LoadMap();
  EXPECT_DOUBLE_EQ(map->GetWaypointXODR(1u, -1, 5.0f)->GetLaneWidth(), 3.5);
  EXPECT_DOUBLE_EQ(map->GetGeoReference().latitude, 49.5);
}

TEST_F(OpenDriveNumericLocale, integers_and_missing_values_control) {
  pugi::xml_document xml;
  ASSERT_TRUE(xml.load_string("<root count='12'/>"));
  EXPECT_EQ(xml.child("root").attribute("count").as_int(), 12);
  EXPECT_DOUBLE_EQ(xml.child("root").attribute("missing").as_double(7.25), 7.25);
}

TEST_F(OpenDriveNumericLocale, parsing_retains_calling_numeric_locale_control) {
  const auto before = std::strtod("3,5", nullptr);
  pugi::xml_document xml;
  ASSERT_TRUE(xml.load_string("<root value='3.5'/>"));
  xml.child("root").attribute("value").as_double();
  LoadMap();
  EXPECT_STREQ(std::localeconv()->decimal_point, ",");
  EXPECT_DOUBLE_EQ(std::strtod("3,5", nullptr), before);
  EXPECT_DOUBLE_EQ(before, 3.5);
}

TEST_F(OpenDriveNumericLocale, concurrent_callers_keep_independent_locales) {
  std::atomic<size_t> correct{0u};
  const auto worker = [&](bool comma) {
    ScopedNumericLocale locale;
    if (!(comma ? locale.EnableCommaDecimal() : locale.Enable("C"))) {
      return;
    }
    pugi::xml_document xml;
    if (xml.load_string("<root value='3.5'/>")) {
      const auto value = xml.child("root").attribute("value").as_double();
      if (value == 3.5 && std::string(std::localeconv()->decimal_point) == (comma ? "," : ".")) {
        ++correct;
      }
    }
  };
  std::thread first(worker, true);
  std::thread second(worker, false);
  first.join();
  second.join();
  EXPECT_EQ(correct.load(), 2u);
}

TEST_F(OpenDriveNumericLocale, c_numeric_format_semantics_control) {
  ASSERT_TRUE(_locale.Enable("C"));
  pugi::xml_document xml;
  ASSERT_TRUE(xml.load_string(
      "<root prefix=' +3.5tail' hex='0x1.8p1' positive='inf' negative='-inf' "
      "nan='nan' empty='' overflow='1e9999' underflow='1e-9999'/>"));
  const auto node = xml.child("root");
  EXPECT_DOUBLE_EQ(node.attribute("prefix").as_double(), 3.5);
  EXPECT_DOUBLE_EQ(node.attribute("hex").as_double(), 3.0);
  EXPECT_EQ(node.attribute("positive").as_double(), std::numeric_limits<double>::infinity());
  EXPECT_EQ(node.attribute("negative").as_double(), -std::numeric_limits<double>::infinity());
  EXPECT_TRUE(std::isnan(node.attribute("nan").as_double()));
  EXPECT_DOUBLE_EQ(node.attribute("empty").as_double(7.0), 0.0);
  EXPECT_EQ(node.attribute("overflow").as_double(), std::numeric_limits<double>::infinity());
  EXPECT_DOUBLE_EQ(node.attribute("underflow").as_double(), 0.0);
}
