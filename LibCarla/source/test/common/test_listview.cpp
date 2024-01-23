// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ListView.h>

#include <array>
#include <cstring>
#include <list>
#include <set>
#include <string>
#include <vector>

using carla::MakeListView;

template <typename Iterator>
static void TestSequence(carla::ListView<Iterator> view) {
  int count = 0;
  for (auto &&x : view) {
    ASSERT_EQ(x, count);
    ++count;
  }
  ASSERT_EQ(count, 6);
}

TEST(listview, sequence) {
  int array[] = {0, 1, 2, 3, 4, 5};
  TestSequence(MakeListView(array));
  std::array<int, 6u> std_array = {0, 1, 2, 3, 4, 5};
  TestSequence(MakeListView(std_array));
  std::vector<int> vector = {0, 1, 2, 3, 4, 5};
  TestSequence(MakeListView(vector));
  std::list<int> list = {0, 1, 2, 3, 4, 5};
  TestSequence(MakeListView(list));
  std::set<int> set = {0, 1, 2, 3, 4, 5};
  TestSequence(MakeListView(set));
}

TEST(listview, string) {
  std::string str = "Hello list view!";
  std::string result;
  for (char c : MakeListView(str)) {
    result += c;
  }
  ASSERT_EQ(result, str);
  char hello[6u] = {0};
  auto begin = std::begin(hello);
  for (char c : MakeListView(str.begin(), str.begin() + 5u)) {
    *begin = c;
    ++begin;
  }
  ASSERT_EQ(std::strcmp(hello, "Hello"), 0);
}
