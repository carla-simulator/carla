// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/geom/Mesh.h>

#include <cstdlib>
#include <fstream>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

#include "test.h"

namespace {
using carla::geom::Mesh;

class CommaDecimal : public std::numpunct<char> {
 protected:
  char do_decimal_point() const override { return ','; }
};

class ScopedCommaLocale {
 public:
  ScopedCommaLocale() : _previous(std::locale()) {
    std::locale::global(std::locale(std::locale::classic(), new CommaDecimal));
  }
  ~ScopedCommaLocale() { std::locale::global(_previous); }

 private:
  std::locale _previous;
};

Mesh LocaleTriangle() {
  return Mesh({{1.25f, 2.5f, 3.75f}, {4.5f, 2.5f, 3.75f}, {1.25f, 5.625f, 4.125f}},
              {{.25f, .5f, .75f}, {.5f, .75f, .25f}, {.75f, .25f, .5f}}, {1u, 2u, 3u},
              {{.25f, .5f}, {.5f, .75f}, {.75f, .25f}});
}

std::string CommaExport(bool recast) {
  const auto mesh = LocaleTriangle();
  ScopedCommaLocale locale;
  return recast ? mesh.GenerateOBJForRecast() : mesh.GenerateOBJ();
}

std::vector<std::vector<float>> Records(const std::string &document, const std::string &wanted) {
  std::vector<std::vector<float>> records;
  std::istringstream input(document);
  input.imbue(std::locale::classic());
  std::string line;
  while (std::getline(input, line)) {
    std::istringstream row(line);
    row.imbue(std::locale::classic());
    std::string tag;
    row >> tag;
    if (tag != wanted) {
      continue;
    }
    std::vector<float> values;
    float value;
    while (row >> value) {
      values.push_back(value);
    }
    records.push_back(values);
  }
  return records;
}

void CheckDocument(const std::string &document, bool recast) {
  const std::vector<std::vector<float>> positions =
      recast ? std::vector<std::vector<float>>{{1.25f, 3.75f, 2.5f},
                                               {4.5f, 3.75f, 2.5f},
                                               {1.25f, 4.125f, 5.625f}}
             : std::vector<std::vector<float>>{
                   {1.25f, 2.5f, 3.75f}, {4.5f, 2.5f, 3.75f}, {1.25f, 5.625f, 4.125f}};
  EXPECT_EQ(Records(document, "v"), positions);
  if (!recast) {
    EXPECT_EQ(Records(document, "vt"),
              (std::vector<std::vector<float>>{{.25f, .5f}, {.5f, .75f}, {.75f, .25f}}));
    EXPECT_EQ(
        Records(document, "vn"),
        (std::vector<std::vector<float>>{{.25f, .5f, .75f}, {.5f, .75f, .25f}, {.75f, .25f, .5f}}));
  }
}

void SaveCommaExport(const std::string &document, bool recast) {
  if (const auto *folder = std::getenv("CARLA_OBJ_LOCALE_EXPORT_DIR")) {
    std::ofstream(std::string(folder) + (recast ? "/recast.obj" : "/mesh.obj")) << document;
  }
}
}  // namespace

TEST(mesh_numeric_locale, obj_uses_standard_decimal_records_under_comma_locale) {
  const auto document = CommaExport(false);
  CheckDocument(document, false);
  SaveCommaExport(document, false);
}

TEST(mesh_numeric_locale, recast_obj_uses_standard_decimal_records_under_comma_locale) {
  const auto document = CommaExport(true);
  CheckDocument(document, true);
  SaveCommaExport(document, true);
}

TEST(mesh_numeric_locale, ordinary_obj_control) {
  CheckDocument(LocaleTriangle().GenerateOBJ(), false);
}

TEST(mesh_numeric_locale, ordinary_recast_obj_control) {
  CheckDocument(LocaleTriangle().GenerateOBJForRecast(), true);
}

TEST(mesh_numeric_locale, exporters_do_not_change_the_callers_global_locale) {
  const auto previous = std::locale();
  bool retained;
  {
    ScopedCommaLocale locale;
    const auto expected = std::locale();
    const auto mesh = LocaleTriangle();
    mesh.GenerateOBJ();
    mesh.GenerateOBJForRecast();
    retained = std::locale() == expected;
  }
  EXPECT_TRUE(retained);
  EXPECT_EQ(std::locale(), previous);
}
